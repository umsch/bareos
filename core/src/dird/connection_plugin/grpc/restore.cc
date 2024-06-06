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

#include "restore.h"

using grpc::ServerContext;
using grpc::Status;

template <typename... Ts> void ignore(Ts&&...) {}

struct restore_session {
  restore_session_handle* handle{nullptr};
};


template <typename Callback>
bool kv_cb_helper(void* user, const char* key, const char* value)
{
  auto* cb = reinterpret_cast<Callback*>(user);
  return (*cb)(key, value);
}

template <typename Callback>
bool file_callback_helper(void* user, file_status status)
{
  auto* cb = reinterpret_cast<Callback*>(user);
  return (*cb)(status);
}

class RestoreImpl : public Restore::Service {
 public:
  RestoreImpl(restore_capability rc) : cap{rc} {}

 private:
  Status Begin(ServerContext* context,
               const BeginRequest* request,
               BeginResponse* response) override
  {
    ignore(context, request);

    std::string key = std::to_string(sessions.size());

    auto [iter, inserted] = sessions.try_emplace(std::move(key));

    if (!inserted) {
      return Status(grpc::StatusCode::UNKNOWN, "Duplicate session keys.");
    }

    auto* session = cap.create_restore_session();

    if (!session) {
      sessions.erase(iter);
      return Status(grpc::StatusCode::UNKNOWN, "Session could not be created.");
    }

    if (!cap.start_from_jobids(session, request->jobids().size(),
                               request->jobids().data(),
                               request->select_parents())) {
      sessions.erase(iter);
      const char* error = cap.error_string(session);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
      cap.abort_restore_session(session);
    }

    iter->second.handle = session;

    response->set_token(iter->first);
    return Status::OK;
  }
  Status Commit(ServerContext* context,
                const CommitRequest* request,
                CommitResponse* response) override
  {
    ignore(context, response);
    auto& key = request->token();
    auto found = sessions.find(key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    job_started_info info;
    if (!cap.commit_restore_session(handle, &info)) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    response->set_jobid(info.jobid);

    return Status::OK;
  }
  Status Abort(ServerContext* context,
               const AbortRequest* request,
               AbortResponse* response) override
  {
    ignore(context, response);

    auto& key = request->token();

    if (auto found = sessions.find(key); found != sessions.end()) {
      cap.abort_restore_session(found->second.handle);
      sessions.erase(found);
      return Status::OK;
    } else {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }
  }

  Status FinishSelection(ServerContext* context,
                         const FinishSelectionRequest* request,
                         FinishSelectionResponse* response) override
  {
    ignore(context, request, response);

    auto& key = request->token();

    auto found = sessions.find(key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    if (!cap.finish_selection(handle, request->has_bootstrappath()
                                          ? request->bootstrappath().c_str()
                                          : nullptr)) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    const char* bsr = cap.get_bootstrap_path(handle);
    if (!bsr) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    response->set_bootstrappath(bsr);
    return Status::OK;
  }

  Status ChangeDirectory(ServerContext* context,
                         const ChangeDirectoryRequest* request,
                         ChangeDirectoryResponse* response) override
  {
    ignore(context);

    auto& key = request->token();

    auto found = sessions.find(key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    if (!cap.change_directory(handle, request->directory().c_str())) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    auto* current_dir = cap.current_directory(handle);

    if (!current_dir) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    response->set_directory(current_dir);
    return Status::OK;
  }

  Status ListFiles(ServerContext* context,
                   const ListFilesRequest* request,
                   grpc::ServerWriter<File>* response) override
  {
    ignore(context);

    auto& key = request->token();

    auto found = sessions.find(key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    auto lambda = [response](file_status status) -> bool {
      File f;
      f.set_name(status.name);
      f.set_marked(status.marked);
      response->Write(std::move(f));
      return true;
    };

    if (!cap.list_files(handle, file_callback_helper<decltype(lambda)>,
                        &lambda)) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    return Status::OK;
  }

  Status ChangeMarkedStatus(ServerContext* context,
                            const ChangeMarkedRequest* request,
                            ChangeMarkedResponse* response) override
  {
    ignore(context, response);

    auto& key = request->token();

    auto found = sessions.find(key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    bool mark = request->mark();
    auto& regex = request->regex();

    size_t num_affected;

    auto lambda = [&num_affected](file_status) -> bool {
      num_affected += 1;
      return true;
    };

    if (!cap.mark_unmark(handle, regex.c_str(), mark,
                         file_callback_helper<decltype(lambda)>, &lambda)) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    response->set_affected_count(num_affected);
    return Status::OK;
  }

  Status ListSessions(ServerContext* context,
                      const ListSessionsRequest* request,
                      ListSessionsResponse* response) override
  {
    ignore(context, request);

    auto* array = response->mutable_tokens();
    for (auto& [key, _] : sessions) {
      auto* str = array->Add();
      if (!str) { return Status(grpc::StatusCode::UNKNOWN, "Internal error."); }
      *str = key;
    }

    return Status::OK;
  }

  Status SetOptions(ServerContext* context,
                    const SetOptionsRequest* request,
                    SetOptionsResponse* response) override
  {
    ignore(context);

    auto& session_key = request->token();

    auto found = sessions.find(session_key);
    if (found == sessions.end()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT,
                    "No session with that key.");
    }

    auto& session = found->second;
    auto* handle = session.handle;

    if (request->has_job()) {
      if (!cap.set_restore_job(handle, request->job().c_str())) {
        const char* error = cap.error_string(handle);
        return Status(grpc::StatusCode::UNKNOWN,
                      error ? error : "Internal error.");
      }
    }
    if (request->has_client()) {
      if (!cap.set_restore_client(handle, request->client().c_str())) {
        const char* error = cap.error_string(handle);
        return Status(grpc::StatusCode::UNKNOWN,
                      error ? error : "Internal error.");
      }
    }
    if (request->has_catalog()) {
      if (!cap.set_catalog(handle, request->catalog().c_str())) {
        const char* error = cap.error_string(handle);
        return Status(grpc::StatusCode::UNKNOWN,
                      error ? error : "Internal error.");
      }
    }

    auto lambda = [map = response->mutable_current_options()](const char* key,
                                                              const char* val) {
      auto [_, inserted] = map->insert({key, val});
      return inserted;
    };
    if (!cap.enumerate_options(handle, kv_cb_helper<decltype(lambda)>,
                               &lambda)) {
      const char* error = cap.error_string(handle);
      return Status(grpc::StatusCode::UNKNOWN,
                    error ? error : "Internal error.");
    }

    return Status::OK;
  }

  std::unordered_map<std::string, restore_session> sessions;

  restore_capability cap;
};

std::unique_ptr<Restore::Service> MakeRestoreService(restore_capability cap)
{
  return std::make_unique<RestoreImpl>(cap);
}
