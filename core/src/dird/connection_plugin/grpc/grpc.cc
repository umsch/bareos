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

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <string>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

template <typename T> void ignore(T&&) {}

// grpcurl -plaintext -format json -d '{ "username": "admin", "password": "123"
// }' 0.0.0.0:34343 Greeter/Authenticate


#include "dird/connection_plugin/plugin.h"
#include "dird/connection_plugin/list_clients.h"

const bareos_api* bareos;
std::unique_ptr<Server> server;
std::unique_ptr<BareosDirector::Service> service;

auto QueryCapability(bareos_capability cap, size_t bufsize, void* buffer)
{
  return bareos->query(cap, bufsize, buffer);
}

list_client_capability lcc;

bool loadPlugin(const bareos_api* bareos_)
{
  if (bareos_->size != sizeof(*bareos_)) { return false; }
  bareos = bareos_;

  if (!QueryCapability(CAP_ListClients, sizeof(lcc), &lcc)) { return false; }

  return true;
}

class DirectorServiceImpl final : public BareosDirector::Service {
  Status ListClients(ServerContext* context,
                     const ListClientsRequest* request,
                     ServerWriter<ClientListing>* writer) override;
};

bool Start(int port)
{
  if (port < 0) { return false; }

  try {
    std::string server_address = "0.0.0.0:" + std::to_string(port);

    service = std::make_unique<DirectorServiceImpl>();

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(service.get());
    // Finally assemble the server.
    server = builder.BuildAndStart();

    return true;
  } catch (...) {
    return false;
  }
}

bool unloadPlugin()
{
  server->Shutdown();
  server->Wait();

  // delete completion queues here
  server.reset();
  service.reset();

  return true;
}

const char* agplv3 = "...";

plugin_api CONN_PLUGIN_API_SYMBOL_NAME = {
    .size = sizeof(plugin_api),
    .license = agplv3,
    .load = &loadPlugin,
    .unload = &unloadPlugin,
    .start = &Start,
};

template <typename Callback> bool ListClients(Callback&& callback)
{
  return lcc.list_clients(
      +[](const client* Client, void* user) -> bool {
        auto* cb = reinterpret_cast<Callback*>(user);
        return (*cb)(Client);
      },
      &callback);
}


Status DirectorServiceImpl::ListClients(ServerContext* context,
                                        const ListClientsRequest* request,
                                        ServerWriter<ClientListing>* writer)
{
  ignore(context);
  ignore(request);
  ignore(writer);

  if (!::ListClients([writer](const client* Client) -> bool {
        ignore(Client);
        return false;
      })) {
    return Status(grpc::StatusCode::UNKNOWN, "Internal error");
  }

  return grpc::Status::OK;
}
