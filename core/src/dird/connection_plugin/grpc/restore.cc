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

  auto get_session(const RestoreSession& sess)
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
                       "No session with that key.");
    }

    std::optional session = found->second.try_lock();
    if (!session) {
      throw grpc_error(grpc::StatusCode::UNAVAILABLE,
                       "session already in use.");
    }
    return std::move(session).value();
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

  Status Begin(ServerContext*,
               const BeginRequest*,
               BeginResponse* response) override
  {
    ignore(response);
    return Status::OK;
  }

  Status Run(ServerContext*,
             const RunRequest* request,
             RunResponse* response) override
  {
    ignore(request, response);
    return Status::OK;
  }

  Status Cancel(ServerContext*,
                const CancelRequest* request,
                CancelResponse*) override
  {
    ignore(request);
    return Status::OK;
  }

  Status ChangeDirectory(ServerContext*,
                         const ChangeDirectoryRequest* request,
                         ChangeDirectoryResponse* response) override
  {
    try {
      auto session = get_session(request->session());
      auto* handle = session->handle;

      plugin_call(cap.change_directory, handle,
                  request->directory().path().c_str());

      auto* current_dir = plugin_call(cap.current_directory, handle);

      Path curdir;
      *curdir.mutable_path() = std::move(current_dir);
      *response->mutable_current_directory() = std::move(curdir);

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
      auto session = get_session(request->session());

      auto* handle = session->handle;

      auto lambda = [response](file_status status) -> bool {
        File f;
        f.set_name(status.name);
        f.set_marked(status.marked);
        response->Write(std::move(f));
        return true;
      };

      if (!cap.list_files(handle, c_callback<decltype(lambda)>, &lambda)) {
        const char* error = cap.error_string(handle);
        return Status(grpc::StatusCode::UNKNOWN,
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
      auto session = get_session(request->session());
      auto* handle = session->handle;

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

      response->set_affected_count(num_affected);
      return Status::OK;
    } catch (const grpc_error& err) {
      return err.status;
    }
  }


  // TODO: sessions also needs to be (rw-)synchronized
  synchronized<std::unordered_map<std::string, synchronized<restore_session>>,
               std::timed_mutex>
      sessions;

  restore_capability cap;
};

std::unique_ptr<Restore::Service> MakeRestoreService(restore_capability cap)
{
  return std::make_unique<RestoreImpl>(cap);
}
