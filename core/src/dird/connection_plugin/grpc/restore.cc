/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2024-2024 Bareos GmbH & Co. KG

   This program is Free Software; you can redistribute it and/or
   modify it under the terms of version three of the GNU Affero General Public
   License as published by the Free Software Foundation and included
   in the file LICENSE.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "grpc.h"
#include "lib/thread_util.h"
#include "restore.grpc.pb.h"

#include <thread>

using grpc::ServerContext;
using grpc::Status;

using namespace bareos::restore;

FileType bareos_to_grpc_ft(bareos_file_type bft)
{
  switch (bft) {
    case BFT_FILE:
      return FileType::FILE;
    case BFT_DIR_NOT_BACKED_UP:
      return FileType::DIRECTORY_NOT_BACKED_UP;
    case BFT_DIR:
      return FileType::DIRECTORY;
  }

  throw grpc_error(grpc::StatusCode::UNKNOWN, "Unknown bareos file type.");
}

namespace restore_opts_util {
void to_bareos(restore_options* bareos, const RestoreOptions* grpc)
{
  if (grpc->has_replace()) {
    switch (grpc->replace()) {
      case ReplaceType::ALWAYS: {
        bareos->replace = REPLACE_FILE_ALWAYS;
      } break;
      case ReplaceType::NEVER: {
        bareos->replace = REPLACE_FILE_NEVER;
      } break;
      case ReplaceType::IF_OLDER: {
        bareos->replace = REPLACE_FILE_IFOLDER;
      } break;
      case ReplaceType::IF_NEWER: {
        bareos->replace = REPLACE_FILE_IFNEWER;
      } break;
      default: {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "bad replace option.");
      } break;
    }
  }

  if (grpc->has_restore_job()) {
    bareos->restore_job = grpc->restore_job().name().c_str();
  }

  if (grpc->has_restore_client()) {
    bareos->restore_client = grpc->restore_client().name().c_str();
  }

  if (grpc->has_restore_location()) {
    bareos->restore_location = grpc->restore_location().c_str();
  }
}

void to_grpc(RestoreOptions* grpc, const restore_options* bareos)
{
  switch (bareos->replace) {
    case REPLACE_FILE_DEFAULT: {
    } break;
    case REPLACE_FILE_ALWAYS: {
      grpc->set_replace(ReplaceType::ALWAYS);
    } break;
    case REPLACE_FILE_IFNEWER: {
      grpc->set_replace(ReplaceType::IF_NEWER);
    } break;
    case REPLACE_FILE_IFOLDER: {
      grpc->set_replace(ReplaceType::IF_OLDER);
    } break;
    case REPLACE_FILE_NEVER: {
      grpc->set_replace(ReplaceType::NEVER);
    } break;
  }

  if (bareos->restore_job) {
    grpc->mutable_restore_job()->set_name(bareos->restore_job);
  }

  if (bareos->restore_client) {
    grpc->mutable_restore_client()->set_name(bareos->restore_client);
  }

  if (bareos->restore_location) {
    grpc->set_restore_location(bareos->restore_location);
  }
}
};  // namespace restore_opts_util

class session_manager {
 public:
  session_manager(restore_capability* restore) : cap{restore} {}

  class handle {
   public:
    handle() = default;
    handle(session_manager& man,
           restore_session_handle* session,
           std::atomic<restore_session_handle*>* place,
           const std::string* tk)
        : manager{&man}, sess{session}, origin{place}, token{tk}
    {
    }
    handle(const handle&) = delete;
    handle& operator=(const handle& other) = delete;

    handle(handle&& other) { *this = std::move(other); }
    handle& operator=(handle&& other)
    {
      manager = std::exchange(other.manager, nullptr);
      sess = std::exchange(other.sess, nullptr);
      origin = std::exchange(other.origin, nullptr);
      token = std::exchange(other.token, {});
      return *this;
    }

    restore_session_handle* operator*() { return sess; }
    restore_session_handle* operator->() { return sess; }

    void Remove()
    {
      manager->Remove(*token, sess, origin);
      sess = nullptr;
      origin = nullptr;
      token = nullptr;
      manager = nullptr;
    }

    restore_session_handle* Bareos() { return sess; }
    const std::string& Token() { return *token; }

    operator bool() { return sess != nullptr; }

    ~handle() noexcept(false)
    {
      if (sess) {
        if (origin->exchange(sess) != nullptr) {
          throw grpc_error(grpc::StatusCode::UNKNOWN,
                           "Somehow unavailable session was available.");
        }
      }
    }

   private:
    session_manager* manager{nullptr};
    restore_session_handle* sess{nullptr};
    std::atomic<restore_session_handle*>* origin{nullptr};
    const std::string* token{};
  };

  std::vector<std::string> Tokens()
  {
    std::optional locked = sessions.try_lock(lock_interval);

    if (!locked) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Could not acquire sessions lock.");
    }

    auto& map = locked.value();

    std::vector<std::string> v;
    v.reserve(map->size());

    for (auto& [str, _] : *map) { v.push_back(str); }

    return v;
  }

  handle Make()
  {
    std::optional locked = sessions.try_lock(lock_interval);

    if (!locked) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Could not acquire sessions lock.");
    }

    auto& map = locked.value();
    auto id = id_counter.fetch_add(1);
    auto str = std::to_string(id);

    // session is available immediately, so we simply create this
    // with nullptr immediately
    auto [iter, inserted] = map->try_emplace(std::move(str));

    if (!inserted) {
      throw grpc_error(grpc::StatusCode::UNKNOWN,
                       "Could not create session (bad id).");
    }

    auto* bareos_handle = cap->create_restore_session();

    if (!bareos_handle) {
      map->erase(iter);
      throw grpc_error(grpc::StatusCode::UNKNOWN, "Could not create session.");
    }

    return make_handle(iter->first, bareos_handle, &iter->second.sess);
  }

  // throws exception on error!
  handle Take(const std::string& key)
  {
    std::optional avail = sessions.try_lock(lock_interval);

    if (!avail) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Could not acquire sessions lock.");
    }

    auto& map = avail.value();

    auto found = map->find(key);
    if (found == map->end()) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Session does not exist.");
    }

    auto* ptr = found->second.sess.exchange(nullptr);
    if (!ptr) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Session is not available.");
    }

    return make_handle(key, ptr, &found->second.sess);
  }

 private:
  struct session_entry {
    std::atomic<restore_session_handle*> sess;

    session_entry() = default;
    session_entry(restore_session_handle* h) : sess{h} {}
  };

  void Remove(const std::string& token,
              restore_session_handle* hndl,
              std::atomic<restore_session_handle*>* origin)
  {
    // the precondition is that session is currently _NOT_ available,
    // so we do not need to remove it from the available map

    auto s = sessions.lock();
    auto& map = s.get();

    auto found = map.find(token);
    if (found == map.end()) {
      throw grpc_error(grpc::StatusCode::UNKNOWN,
                       "Trying to delete non existing session.");
    }

    if (&found->second.sess != origin) {
      throw grpc_error(grpc::StatusCode::UNKNOWN,
                       "key-value mismatch while trying to delete session.");
    }

    cap->finish_restore_session(hndl);

    map.erase(found);
  }

  handle make_handle(const std::string& token,
                     restore_session_handle* sess,
                     std::atomic<restore_session_handle*>* origin)
  {
    return handle{*this, sess, origin, &token};
  }
  constexpr static auto lock_interval = std::chrono::milliseconds(30);

  template <typename T> using timed_sync = synchronized<T, std::timed_mutex>;

  std::atomic<size_t> id_counter;
  timed_sync<std::unordered_map<std::string, session_entry>> sessions;
  restore_capability* cap;
};

class RestoreImpl : public Restore::Service {
 public:
  RestoreImpl(restore_capability rc) : cap{rc}, sessions{&cap} {}

 private:
  template <typename F, typename Handle, typename... Args>
  auto plugin_call(F* fun, Handle&& handle, Args&&... args)
  {
    auto res = fun(handle, std::forward<Args>(args)...);
    if (!res) {
      const char* error = cap.error_string(handle);
      throw grpc_error(grpc::StatusCode::UNKNOWN,
                       error ? error : "Unknown error!");
    }

    return res;
  }

  session_manager::handle new_session() { return sessions.Make(); }

  session_manager::handle get_session(const RestoreSession& sess)
  {
    return sessions.Take(sess.token());
  }

  Status ListSessions(ServerContext*,
                      const ListSessionsRequest*,
                      ListSessionsResponse* response) override
  {
    auto* array = response->mutable_sessions();

    std::vector session_tokens = sessions.Tokens();

    for (auto& token : session_tokens) {
      RestoreSession sess;
      sess.set_token(token);
      array->Add(std::move(sess));
    }

    return Status::OK;
  }

  Status Begin(ServerContext*,
               const BeginRequest* request,
               BeginResponse* response) override
  {
    try {
      if (request->find_job_chain()) {
        throw grpc_error(grpc::StatusCode::UNIMPLEMENTED,
                         "This option is currently not implemented.");
      }

      auto jobid = request->backup_job().jobid();

      auto handle = new_session();

      if (!handle) {
        throw grpc_error(grpc::StatusCode::INTERNAL,
                         "Could not create restore session.");
      }

      if (!cap.start_from_jobids(handle.Bareos(), 1, &jobid,
                                 request->find_job_chain())) {
        const char* error = cap.error_string(handle.Bareos());
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      RestoreSession id;
      id.mutable_token()->assign(handle.Token());
      *response->mutable_session() = std::move(id);

    } catch (const grpc_error& err) {
      return err.status;
    }
    return Status::OK;
  }

  Status Run(ServerContext*,
             const RunRequest* request,
             RunResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      job_started_info info;

      if (!cap.create_restore_job(handle.Bareos(), &info)) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Could not start restore job.");
      }

      handle.Remove();

      bareos::common::Job job;
      job.set_jobid(info.jobid);
      *response->mutable_jobid() = std::move(job);

    } catch (const grpc_error& err) {
      return err.status;
    }
    return Status::OK;
  }

  Status Cancel(ServerContext*,
                const CancelRequest* request,
                CancelResponse*) override
  {
    try {
      auto handle = get_session(request->session());

      handle.Remove();

    } catch (const grpc_error& err) {
      return err.status;
    }
    return Status::OK;
  }

  Status PathToFile(ServerContext*,
                    const PathToFileRequest* request,
                    PathToFileResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      file_status status;
      plugin_call(cap.path_to_file, handle.Bareos(),
                  request->p().path().c_str(), &status);

      File f;
      f.mutable_id()->set_value(status.id);
      f.set_name(status.name);
      f.set_marked(status.marked);
      f.set_type(bareos_to_grpc_ft(status.type));
      *response->mutable_f() = f;

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status ChangeDirectory(ServerContext*,
                         const ChangeDirectoryRequest* request,
                         ChangeDirectoryResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      plugin_call(cap.change_directory, handle.Bareos(),
                  request->directory().value());

      auto* segments = response->mutable_current_dir()->mutable_segments();

      auto lambda = [segments](file_status status) -> bool {
        File f;
        f.mutable_id()->set_value(status.id);
        f.set_name(status.name);
        f.set_marked(status.marked);
        f.set_type(bareos_to_grpc_ft(status.type));
        segments->Add(std::move(f));
        return true;
      };

      if (!cap.path_segments_of(handle.Bareos(), request->directory().value(),
                                c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle.Bareos());
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status CurrentDirectory(ServerContext*,
                          const CurrentDirectoryRequest* request,
                          CurrentDirectoryResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      auto* segments = response->mutable_current_dir()->mutable_segments();

      size_t current_id = std::numeric_limits<size_t>::max();
      plugin_call(cap.current_directory, handle.Bareos(), &current_id);

      auto lambda = [segments](file_status status) -> bool {
        File f;
        f.mutable_id()->set_value(status.id);
        f.set_name(status.name);
        f.set_marked(status.marked);
        f.set_type(bareos_to_grpc_ft(status.type));
        segments->Add(std::move(f));
        return true;
      };

      if (!cap.path_segments_of(handle.Bareos(), current_id,
                                c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle.Bareos());
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status ListFiles(ServerContext*,
                   const ListFilesRequest* request,
                   grpc::ServerWriter<File>* response) override
  {
    size_t root;
    size_t* root_ptr{nullptr};

    if (request->has_root()) {
      root = request->root().value();
      root_ptr = &root;
    }


    try {
      auto handle = get_session(request->session());

      auto lambda = [response](file_status status) -> bool {
        File f;
        f.mutable_id()->set_value(status.id);
        f.set_name(status.name);
        f.set_marked(status.marked);
        f.set_type(bareos_to_grpc_ft(status.type));
        response->Write(std::move(f));
        return true;
      };

      if (!cap.list_files(handle.Bareos(), root_ptr,
                          c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle.Bareos());
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status ChangeMarkedStatus(ServerContext*,
                            const ChangeMarkedRequest* request,
                            ChangeMarkedResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      auto action = request->action();
      auto id = request->affected_id();
      auto recursive = request->recursive();

      bool mark = [&]() {
        switch (action) {
          case MarkAction::MARK:
            return true;
          case MarkAction::UNMARK:
            return false;
          default:
            throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                             "action must either be mark or unmark.");
        }
      }();

      if (!cap.mark_unmark(handle.Bareos(), id.value(), mark, recursive)) {
        const char* error = cap.error_string(handle.Bareos());
        return Status(grpc::StatusCode::UNKNOWN,
                      error ? error : "Internal error.");
      }

      (void)response;
      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status GetState(ServerContext*,
                  const GetStateRequest* request,
                  GetStateResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      bareos_session_state s{};
      plugin_call(cap.current_session_state, handle.Bareos(), &s);

      RestoreOptions opts;
      restore_opts_util::to_grpc(&opts, &s.options);

      auto* state = response->mutable_state();
      state->set_files_marked_count(s.marked_count);
      state->mutable_catalog()->set_name(s.catalog_name);
      state->mutable_backup_job()->set_jobid(s.backup_id);
      *state->mutable_restore_options() = std::move(opts);

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  Status UpdateState(ServerContext*,
                     const UpdateStateRequest* request,
                     UpdateStateResponse* response) override
  {
    try {
      auto handle = get_session(request->session());

      restore_options options{};
      restore_opts_util::to_bareos(&options, &request->new_options());

      plugin_call(cap.update_restore_state, handle.Bareos(), &options);

      bareos_session_state s{};

      plugin_call(cap.current_session_state, handle.Bareos(), &s);

      RestoreOptions opts;
      restore_opts_util::to_grpc(&opts, &s.options);

      auto* state = response->mutable_state();
      state->set_files_marked_count(s.marked_count);
      state->mutable_catalog()->set_name(s.catalog_name);
      state->mutable_backup_job()->set_jobid(s.backup_id);
      *state->mutable_restore_options() = std::move(opts);

      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  restore_capability cap;
  session_manager sessions;
};

std::unique_ptr<Restore::Service> MakeRestoreService(restore_capability cap)
{
  return std::make_unique<RestoreImpl>(cap);
}
