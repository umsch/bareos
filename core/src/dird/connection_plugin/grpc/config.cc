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

#include "dird/connection_plugin/config.h"
#include "grpc.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace {
bool type_ok(size_t combined_grpc_types, bareos_job_type type)
{
  using namespace bareos::config;
  switch (type) {
    case BJT_BACKUP: {
      return (combined_grpc_types & (1 << BACKUP)) == (1 << BACKUP);
    } break;
    case BJT_RESTORE: {
      return (combined_grpc_types & (1 << RESTORE)) == (1 << RESTORE);
    } break;
  }

  return false;
}
};  // namespace

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
      auto lambda = [clients](const bareos_config_client* data) {
        Client c;
        c.set_name(data->name);
        c.set_address(data->address);
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
    try {
      /* if no filter is set, then we accept everything by default,
       * otherwise we accept nothing by default */
      size_t accepted_types = request->jtf_size() > 0 ? 0 : ~0;

      for (auto& x : request->jtf()) {
        auto grpc_job_type = x.type();

        switch (grpc_job_type) {
          case RESTORE: {
            accepted_types |= (1 << RESTORE);
          } break;
          case BACKUP: {
            accepted_types |= (1 << BACKUP);
          } break;
          default: {
            throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                             "bad jobtype enum value.");
          }
        }
      }
      auto* jobs = response->mutable_jobs();
      auto lambda = [accepted_types, jobs](const bareos_config_job* data) {
        if (type_ok(accepted_types, data->type)) {
          Job j;
          j.set_name(data->name);

          switch (data->type) {
            case BJT_BACKUP: {
              j.set_type(BACKUP);
            } break;
            case BJT_RESTORE: {
              j.set_type(RESTORE);
            } break;
            default: {
              return false;
            }
          }

          switch (data->level) {
            case BJL_NONE: {
              // do nothing
            } break;
            case BJL_FULL: {
              j.set_default_level(FULL);
            } break;
            case BJL_DIFFERENTIAL: {
              j.set_default_level(DIFFERENTIAL);
            } break;
            case BJL_INCREMENTAL: {
              j.set_default_level(INCREMENTAL);
            } break;
            default: {
              return false;
            }
          }

          jobs->Add(std::move(j));
        }
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
      auto lambda = [catalogs](const bareos_config_catalog* data) {
        Catalog c;
        c.set_name(data->name);
        c.set_dbname(data->db_name);
        catalogs->Add(std::move(c));
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
