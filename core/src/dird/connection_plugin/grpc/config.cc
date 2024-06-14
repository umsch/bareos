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

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace bareos::config {
class ConfigImpl final : public Config::Service {
 public:
  ConfigImpl(config_capability cc) : cap{cc} {}

 private:
  Status ListClients(ServerContext*,
                     const ListClientsRequest*,
                     ListClientsResponse* response) override
  {
    try {
      auto* clients = response->mutable_clients();
      auto lambda = [clients](const char* name) {
        Client c;
        c.set_name(name);
        clients->Add(std::move(c));
        return true;
      };

      if (!cap.list_clients(c_callback<decltype(lambda)>, &lambda)) {
        throw grpc_error(grpc::StatusCode::UNKNOWN, "Internal bareos error");
      }
    } catch (const grpc_error& err) {
      return err.status;
    }

    return Status::OK;
  }
  Status ListJobs(ServerContext*,
                  const ListJobsRequest* request,
                  ListJobsResponse* response) override
  {
    ignore(request);
    try {
      auto* jobs = response->mutable_jobs();
      auto lambda = [jobs](const char* name) {
        Job j;
        j.set_name(name);
        jobs->Add(std::move(j));
        return true;
      };

      if (!cap.list_jobs(c_callback<decltype(lambda)>, &lambda)) {
        throw grpc_error(grpc::StatusCode::UNKNOWN, "Internal bareos error");
      }
    } catch (const grpc_error& err) {
      return err.status;
    }

    return Status::OK;
  }
  Status ListCatalogs(ServerContext*,
                      const ListCatalogsRequest*,
                      ListCatalogsResponse* response) override
  {
    try {
      auto* catalogs = response->mutable_catalogs();
      auto lambda = [catalogs](const char* name) {
        Catalog j;
        j.set_name(name);
        catalogs->Add(std::move(j));
        return true;
      };

      if (!cap.list_catalogs(c_callback<decltype(lambda)>, &lambda)) {
        throw grpc_error(grpc::StatusCode::UNKNOWN, "Internal bareos error");
      }
    } catch (const grpc_error& err) {
      return err.status;
    }

    return Status::OK;
  }

  config_capability cap;
};
}  // namespace bareos::config

std::unique_ptr<bareos::config::Config::Service> MakeConfigService(
    config_capability cap)
{
  return std::make_unique<bareos::config::ConfigImpl>(cap);
}
