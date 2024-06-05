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
#include "dird/connection_plugin/list_clients.h"
#include "restore.h"

const bareos_api* bareos;
std::unique_ptr<Server> server;
std::vector<std::unique_ptr<grpc::Service>> services;

auto QueryCapability(bareos_capability cap, size_t bufsize, void* buffer)
{
  return bareos->query(cap, bufsize, buffer);
}

bool loadPlugin(const bareos_api* bareos_)
{
  if (bareos_->size != sizeof(*bareos_)) { return false; }
  bareos = bareos_;

  return true;
}

class SqlImpl final : public Sql::Service {
 public:
  SqlImpl(list_client_capability lcc) : cap{lcc} { Dmsg1(5, "Impl Created"); }

 private:
  Status ListClients(ServerContext* context,
                     const ListClientsRequest* request,
                     ServerWriter<SqlResponse>* writer) override;

  list_client_capability cap;
};


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

    if (list_client_capability lcc;
        QueryCapability(CAP_ListClients, sizeof(lcc), &lcc)) {
      auto sql = std::make_unique<SqlImpl>(lcc);
      builder.RegisterService(sql.get());
      services.emplace_back(std::move(sql));
    }

    if (restore_capability rc; QueryCapability(CAP_Restore, sizeof(rc), &rc)) {
      auto restore = MakeRestoreService(rc);
      builder.RegisterService(restore.get());
      services.emplace_back(std::move(restore));
    }

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
  services.clear();

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

template <typename Callback>
bool sql_callback_helper(size_t num_fields,
                         const char* const fields[],
                         const char* const rows[],
                         void* user)
{
  auto* cb = reinterpret_cast<Callback*>(user);
  return (*cb)(num_fields, fields, rows);
}

template <typename Callback>
bool ListClients(list_client_capability& lcc, Callback&& callback)
{
  return lcc.list_clients(sql_callback_helper<Callback>, &callback);
}

template <typename Callback>
bool ListClient(list_client_capability& lcc,
                const char* name,
                Callback&& callback)
{
  return lcc.list_client(name, sql_callback_helper<Callback>, &callback);
}


Status SqlImpl::ListClients(ServerContext* context,
                            const ListClientsRequest* request,
                            ServerWriter<SqlResponse>* writer)
{
  ignore(context);

  auto cb = [writer](size_t num_fields, const char* const fields[],
                     const char* const rows[]) -> bool {
    SqlResponse response;
    for (size_t i = 0; i < num_fields; ++i) {
      auto [_, inserted] = response.mutable_row()->insert({fields[i], rows[i]});
      if (!inserted) { return false; }
    }
    writer->Write(response);
    return true;
  };
  if (request->has_clientname()) {
    if (!::ListClient(cap, request->clientname().c_str(), std::move(cb))) {
      return Status(grpc::StatusCode::UNKNOWN, "Internal error");
    }
  } else {
    if (!::ListClients(cap, std::move(cb))) {
      return Status(grpc::StatusCode::UNKNOWN, "Internal error");
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));

  return grpc::Status::OK;
}
