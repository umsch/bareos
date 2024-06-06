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

#include "service.grpc.pb.h"
#include "include/baconfig.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <string>
#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

template <typename... Ts> void ignore(Ts&&...) {}

// grpcurl -plaintext -format json -d '{ "username": "admin", "password": "123"
// }' 0.0.0.0:34343 Greeter/Authenticate


#include "dird/connection_plugin/plugin.h"
#include "grpc.h"

namespace {

constexpr time_t default_timeout = 500;

class AuthImpl final : public Authentication::Service {
  Status Login(ServerContext*,
               const LoginRequest* request,
               LoginResponse* response) override
  {
    if (request->username() != "admin" || request->password() != "admin") {
      return Status(grpc::StatusCode::PERMISSION_DENIED, "No");
    }

    std::string sesskey;

    auto* session
        = MakeSession(request->username(), request->password(), sesskey);

    if (!session) { return Status(grpc::StatusCode::UNKNOWN, "No"); }

    if (request->has_timeout()) {
      session->timeout = request->timeout();
    } else {
      session->timeout = default_timeout;
    }

    session->last_action = time(NULL);

    response->set_auth(sesskey);
    response->set_timeout(session->timeout);

    return Status::OK;
  }
  Status Logout(ServerContext*, const LogoutRequest*, LogoutResponse*) override
  {
    return Status::OK;
  }

  struct auth_session {
    time_t timeout;
    time_t last_action;
  };

  std::unordered_map<std::string, auth_session> sessions;

  auth_session* MakeSession(std::string_view user,
                            std::string_view pw,
                            std::string& key)
  {
    (void)user;
    (void)pw;
    (void)key;
    return nullptr;
  }
};


const bareos_api* bareos;
std::unique_ptr<Server> server;
std::vector<std::unique_ptr<grpc::Service>> services;

auto QueryCapability(bareos_capability cap, size_t bufsize, void* buffer)
{
  return bareos->query(cap, bufsize, buffer);
}

bool Start(int port)
{
  if (port < 0) { return false; }

  try {
    std::string server_address = "0.0.0.0:" + std::to_string(port);


    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.

    if (client_capability cc; QueryCapability(CAP_Client, sizeof(cc), &cc)) {
      auto client = MakeClientService(cc);
      builder.RegisterService(client.get());
      services.emplace_back(std::move(client));
    }

    if (restore_capability rc; QueryCapability(CAP_Restore, sizeof(rc), &rc)) {
      auto restore = MakeRestoreService(rc);
      builder.RegisterService(restore.get());
      services.emplace_back(std::move(restore));
    }

    services.emplace_back(new AuthImpl{});

    // Finally assemble the server.
    server = builder.BuildAndStart();

    return true;
  } catch (...) {
    return false;
  }
}

bool loadPlugin(const bareos_api* bareos_)
{
  if (bareos_->size != sizeof(*bareos_)) { return false; }
  bareos = bareos_;

  return true;
}

bool unloadPlugin()
{
  server->Shutdown();
  server->Wait();

  // delete completion queues here
  server.reset();
  services.clear();

  return true;
}

const char* agplv3 = "...";
}  // namespace

plugin_api CONN_PLUGIN_API_SYMBOL_NAME = {
    .size = sizeof(plugin_api),
    .license = agplv3,
    .load = &loadPlugin,
    .unload = &unloadPlugin,
    .start = &Start,
};
