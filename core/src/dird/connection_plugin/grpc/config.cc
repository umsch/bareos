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
#include "config.pb.h"
#include "grpc.h"

#include <optional>

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace bareos::config {

namespace {


bool job_filter(const ::google::protobuf::RepeatedPtrField<
                    ::bareos::config::JobFilter>& filters,
                const Job& job)
{
  // we go through every filter regardless of whether we already know
  // that we do not accept the job, just so we can do some input checking
  auto accept = true;
  for (auto& filter : filters) {
    switch (filter.filter_type_case()) {
      case JobFilter::kType: {
        if (!filter.has_type()) {
          throw grpc_error(grpc::StatusCode::UNKNOWN, "bad protobuf contents");
        }
        if (filter.type().select() != job.type()) { accept = false; }
      } break;
      case JobFilter::FILTER_TYPE_NOT_SET: {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "filter type is not set.");
      } break;
    }
  }
  return accept;
}

std::optional<bareos::config::JobType> bareos_to_grpc_type(bareos_job_type type)
{
  using namespace bareos::config;
  switch (type) {
    case BJT_BACKUP:
      return BACKUP;
    case BJT_COPY:
      return COPY;
    case BJT_RESTORE:
      return RESTORE;

    case BJT_VERIFY:
      return VERIFY;
    case BJT_ADMIN:
      return ADMIN;
    case BJT_MIGRATE:
      return MIGRATE;
    case BJT_CONSOLIDATE:
      return CONSOLIDATE;

    case BJT_SCAN:
      [[fallthrough]];
    case BJT_SYSTEM:
      [[fallthrough]];
    case BJT_ARCHIVE:
      [[fallthrough]];
    case BJT_JOB_COPY:
      [[fallthrough]];
    case BJT_CONSOLE:
      [[fallthrough]];
    case BJT_MIGRATED_JOB:
      return std::nullopt;
  }
  return std::nullopt;
}
};  // namespace

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
        c.mutable_id()->mutable_name()->assign(data->name);
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
      auto* jobs = response->mutable_jobs();
      auto lambda = [&filters = request->filters(),
                     jobs](const bareos_config_job* data) {
        Job j;
        j.mutable_id()->mutable_name()->assign(data->name);
        j.set_name(data->name);

        if (std::optional type = bareos_to_grpc_type(data->type)) {
          j.set_type(type.value());
        } else {
          return false;
        }

        if (!job_filter(filters, j)) { return true; }

        if (data->description) { j.set_description(data->description); }

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
      auto lambda = [catalogs](const bareos_config_catalog* data) {
        Catalog c;
        c.mutable_id()->mutable_name()->assign(data->name);
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

  void CatalogDefinition(const char* name, GetDefinitionResponse* resp)
  {
    auto lambda = [opts = resp->mutable_set_options()](
                      const char* opt, const char* val) -> bool {
      opts->insert({opt, val});
      return true;
    };
    if (!cap.catalog_definition(name, c_callback<decltype(lambda)>, &lambda)) {
      throw grpc_error(grpc::StatusCode::UNKNOWN, "internal bareos error");
    }
  }
  void JobDefinition(const char* name, GetDefinitionResponse* resp)
  {
    auto lambda = [opts = resp->mutable_set_options()](
                      const char* opt, const char* val) -> bool {
      opts->insert({opt, val});
      return true;
    };
    if (!cap.job_definition(name, c_callback<decltype(lambda)>, &lambda)) {
      throw grpc_error(grpc::StatusCode::UNKNOWN, "internal bareos error");
    }
  }
  void ClientDefinition(const char* name, GetDefinitionResponse* resp)
  {
    auto lambda = [opts = resp->mutable_set_options()](
                      const char* opt, const char* val) -> bool {
      opts->insert({opt, val});
      return true;
    };
    if (!cap.client_definition(name, c_callback<decltype(lambda)>, &lambda)) {
      throw grpc_error(grpc::StatusCode::UNKNOWN, "internal bareos error");
    }
  }

  Status GetDefinition(ServerContext*,
                       const GetDefinitionRequest* request,
                       GetDefinitionResponse* response) override
  {
    try {
      if (request->has_catalog()) {
        CatalogDefinition(request->catalog().name().c_str(), response);
      } else if (request->has_job()) {
        JobDefinition(request->job().name().c_str(), response);
      } else if (request->has_client()) {
        ClientDefinition(request->client().name().c_str(), response);
      } else {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "no usable configuration id given");
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
