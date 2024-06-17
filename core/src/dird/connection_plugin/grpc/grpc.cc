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
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "include/baconfig.h"

#include <string>
#include <thread>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

#include "dird/connection_plugin/plugin.h"

namespace {

constexpr time_t default_timeout = 500;

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

    if (restore_capability rc; QueryCapability(CAP_Restore, sizeof(rc), &rc)) {
      auto restore = MakeRestoreService(rc);
      builder.RegisterService(restore.get());
      services.emplace_back(std::move(restore));
    }

    if (config_capability cc; QueryCapability(CAP_Config, sizeof(cc), &cc)) {
      auto config = MakeConfigService(cc);
      builder.RegisterService(config.get());
      services.emplace_back(std::move(config));
    }

    if (database_capability dc;
        QueryCapability(CAP_Database, sizeof(dc), &dc)) {
      auto database = MakeDatabaseService(dc);
      builder.RegisterService(database.get());
      services.emplace_back(std::move(database));
    }

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
