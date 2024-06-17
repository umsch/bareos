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


struct restore_message {
  message_severity severity;
  std::string text;
  time_t timestamp;
};

struct restore_session {
  restore_session_handle* handle;
  std::shared_ptr<synchronized<std::vector<restore_message>>> messages;
};

class RestoreImpl : public Restore::Service {
 public:
  RestoreImpl(restore_capability rc) : cap{rc} {}

 private:
  template <typename F, typename Handle, typename... Args>
  auto plugin_call(F* fun, Handle& handle, Args&&... args)
  {
    auto res = fun(handle, std::forward<Args>(args)...);
    if (!res) {
      const char* error = cap.error_string(handle);
      throw grpc_error(grpc::StatusCode::UNKNOWN,
                       error ? error : "Unknown error!");
    }

    return res;
  }

  bool push_session(const RestoreSession& sess, restore_session&& restore)
  {
    auto sessmap = sessions.lock();

    auto [_, inserted] = sessmap->try_emplace(sess.token(), restore);

    return inserted;
  }

  auto pop_session(const RestoreSession& sess)
  {
    std::optional optsessmap = sessions.try_lock(std::chrono::milliseconds(5));

    if (!optsessmap) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "Too much concurrent use.");
    }

    auto& sessmap = optsessmap.value().get();

    auto found = sessmap.find(sess.token());
    if (found == sessmap.end()) {
      throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                       "No session with that key or session already in use.");
    }

    auto session = std::move(found->second);

    sessmap.erase(found);

    return session;
  }

  Status ListSessions(ServerContext*,
                      const ListSessionsRequest*,
                      ListSessionsResponse* response) override
  {
    auto* array = response->mutable_sessions();

    std::optional optsessmap = sessions.try_lock(std::chrono::milliseconds(5));

    if (!optsessmap) {
      return Status(grpc::StatusCode::UNAVAILABLE, "Too much concurrent use.");
    }

    auto& sessmap = optsessmap.value().get();

    for (auto& [key, _] : sessmap) {
      RestoreSession sess;
      sess.set_token(key);
      array->Add(std::move(sess));
    }

    return Status::OK;
  }

  RestoreSession new_id()
  {
    std::optional all_sess
        = all_sessions.try_lock(std::chrono::milliseconds(5));

    if (!all_sess) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE, "Please try again.");
    }
    RestoreSession id;
    id.set_token("0");  // todo: make this random
    return id;
  }

  void delete_id(const RestoreSession& id)
  {
    (void)id;
    std::optional all_sess
        = all_sessions.try_lock(std::chrono::milliseconds(5));

    if (!all_sess) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE, "Please try again.");
    }
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

      restore_session session;

      auto jobid = request->backup_job().jobid();

      session.handle = cap.create_restore_session();

      if (!session.handle) {
        throw grpc_error(grpc::StatusCode::INTERNAL,
                         "Could not create restore session.");
      }

      if (!cap.start_from_jobids(session.handle, 1, &jobid,
                                 request->find_job_chain())) {
        const char* error = cap.error_string(session.handle);
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      // TODO: make random + some retries
      RestoreSession id = new_id();

      if (!push_session(id, std::move(session))) {
        cap.finish_restore_session(session.handle);
        delete_id(id);
        throw grpc_error(grpc::StatusCode::RESOURCE_EXHAUSTED,
                         "Could not create restore session. Please try again.");
      }


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
      auto session = pop_session(request->session());

      auto* handle = session.handle;

      restore_options options = {};

      const RestoreOptions& opts = request->restore_options();

      if (opts.has_replace()) {
        switch (opts.replace()) {
          case ReplaceType::ALWAYS: {
            options.replace = REPLACE_FILE_ALWAYS;
          } break;
          case ReplaceType::NEVER: {
            options.replace = REPLACE_FILE_NEVER;
          } break;
          case ReplaceType::IF_OLDER: {
            options.replace = REPLACE_FILE_IFOLDER;
          } break;
          case ReplaceType::IF_NEWER: {
            options.replace = REPLACE_FILE_IFNEWER;
          } break;
          default: {
            throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                             "bad replace option.");
          } break;
        }
      }

      if (opts.has_restore_job()) {
        options.restore_job = opts.restore_job().name().c_str();
      }

      if (opts.has_restore_client()) {
        options.restore_client = opts.restore_client().name().c_str();
      }

      if (opts.has_restore_location()) {
        options.restore_location = opts.restore_location().c_str();
      }

      job_started_info info;

      if (!cap.create_restore_job(handle, options, &info)) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Could not start restore job.");
      }

      cap.finish_restore_session(handle);
      delete_id(request->session());

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
      auto session = pop_session(request->session());

      auto* handle = session.handle;

      cap.finish_restore_session(handle);

      delete_id(request->session());
    } catch (const grpc_error& err) {
      return err.status;
    }
    return Status::OK;
  }

  Status ChangeDirectory(ServerContext*,
                         const ChangeDirectoryRequest* request,
                         ChangeDirectoryResponse* response) override
  {
    try {
      auto session = pop_session(request->session());
      auto* handle = session.handle;

      plugin_call(cap.change_directory, handle,
                  request->directory().path().c_str());

      auto* current_dir = plugin_call(cap.current_directory, handle);

      Path curdir;
      *curdir.mutable_path() = std::move(current_dir);
      *response->mutable_current_directory() = std::move(curdir);

      if (!push_session(request->session(), std::move(session))) {
        cap.finish_restore_session(handle);
        throw grpc_error(grpc::StatusCode::ABORTED,
                         "Aborted the current session.");
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
    try {
      auto session = pop_session(request->session());

      auto* handle = session.handle;

      auto lambda = [response](file_status status) -> bool {
        File f;
        f.set_name(status.name);
        f.set_marked(status.marked);
        response->Write(std::move(f));
        return true;
      };

      if (!cap.list_files(handle, c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle);
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         error ? error : "Internal error.");
      }

      if (!push_session(request->session(), std::move(session))) {
        cap.finish_restore_session(handle);
        throw grpc_error(grpc::StatusCode::ABORTED,
                         "Aborted the current session.");
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
      auto session = pop_session(request->session());
      auto* handle = session.handle;

      auto action = request->action();
      auto& regex = request->filter();

      size_t num_affected = 0;

      auto lambda = [&num_affected](file_status) -> bool {
        num_affected += 1;
        return true;
      };

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

      if (!cap.mark_unmark(handle, regex.regex().c_str(), mark,
                           c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle);
        return Status(grpc::StatusCode::UNKNOWN,
                      error ? error : "Internal error.");
      }

      if (!push_session(request->session(), std::move(session))) {
        cap.finish_restore_session(handle);
        throw grpc_error(grpc::StatusCode::ABORTED,
                         "Aborted the current session.");
      }

      response->set_affected_count(num_affected);
      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }

  synchronized<std::vector<std::string>, std::timed_mutex> all_sessions;


  // TODO: sessions also needs to be (rw-)synchronized
  synchronized<std::unordered_map<std::string, restore_session>,
               std::timed_mutex>
      sessions;

  restore_capability cap;
};

std::unique_ptr<Restore::Service> MakeRestoreService(restore_capability cap)
{
  return std::make_unique<RestoreImpl>(cap);
}
