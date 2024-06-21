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

namespace bareos::database {
class DatabaseImpl final : public Database::Service {
 public:
  DatabaseImpl(database_capability cc) : cap{cc} {}

 private:
  struct database {
    database_session* db{nullptr};
    database_capability* cap;

    database(database_session* db_, database_capability* cap_)
        : db{db_}, cap{cap_}
    {
    }

    template <typename F> void list_clients(F&& f)
    {
      if (!db) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Something went wrong (no db is set).\n");
      }

      if (!cap->list_clients(db, c_callback<F>, &f)) {
        const char* err = cap->error_string(db);
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         err ? err : "Something went wrong.\n");
      }
    }

    template <typename F> void list_jobs(F&& f)
    {
      if (!db) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Something went wrong (no db is set).\n");
      }

      if (!cap->list_jobs(db, c_callback<F>, &f)) {
        const char* err = cap->error_string(db);
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         err ? err : "Something went wrong.\n");
      }
    }

    operator bool() const { return db != nullptr; }

    ~database()
    {
      if (db) { cap->close_database(db); }
    }
  };

  database OpenDb(const bareos::config::Catalog& ctlg)
  {
    auto* ptr = cap.open_database(ctlg.name().c_str());
    if (!ptr) {
      // this should never happen
      throw grpc_error(grpc::StatusCode::UNKNOWN, "opening db failed.");
    }

    auto* error = cap.error_string(ptr);

    if (error) {
      cap.close_database(ptr);
      throw grpc_error(grpc::StatusCode::UNKNOWN, error);
    }

    return database{ptr, &cap};
  }

  Status ListClients(ServerContext*,
                     const ListClientsRequest* request,
                     ServerWriter<ListClientsResponse>* response) override
  {
    try {
      auto db = OpenDb(request->catalog());

      db.list_clients([writer = response](size_t field_count,
                                          const char* const* fields,
                                          const char* const* cols) -> bool {
        for (size_t i = 0; i < field_count; ++i) {
          if (strcmp(fields[i], "name") == 0) {
            ListClientsResponse resp;
            resp.mutable_client()->set_name(cols[i]);
            writer->Write(resp);
            return true;
          }
        }

        return false;
      });
    } catch (const grpc_error& err) {
      return err.status;
    }

    return Status::OK;
  }
  Status ListJobs(ServerContext*,
                  const ListJobsRequest* request,
                  ServerWriter<ListJobsResponse>* response) override
  {
    try {
      auto db = OpenDb(request->catalog());

      db.list_jobs([writer = response](size_t field_count,
                                       const char* const* fields,
                                       const char* const* cols) -> bool {
        for (size_t i = 0; i < field_count; ++i) {
          if (strcmp(fields[i], "jobid") == 0) {
            ListJobsResponse resp;
            resp.mutable_job()->set_jobid(std::atoi(cols[i]));
            writer->Write(resp);
            return true;
          }
        }
        return false;
      });
    } catch (const grpc_error& err) {
      return grpc::Status(grpc::StatusCode::UNKNOWN,
                          "HalloHalloHalloHalloHalloHalloHalloHalloHalloHalloHa"
                          "lloHalloHalloHalloHalloHalloHalloHalloHalloHallo",
                          "Test");
    }

    return Status::OK;
  }

  database_capability cap;
};
}  // namespace bareos::database

std::unique_ptr<bareos::database::Database::Service> MakeDatabaseService(
    database_capability cap)
{
  return std::make_unique<bareos::database::DatabaseImpl>(cap);
}
