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

#include <optional>
#include "grpc.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

// Convert standard time string yyyy-mm-dd hh:mm:ss to Unix time
time_t StrToUtime(const char* str)
{
  tm datetime{};
  time_t time;

  char trailinggarbage[16]{""};

  // Check for bad argument
  if (!str || *str == 0) { return 0; }

  if ((sscanf(str, "%u-%u-%u %u:%u:%u%15s", &datetime.tm_year, &datetime.tm_mon,
              &datetime.tm_mday, &datetime.tm_hour, &datetime.tm_min,
              &datetime.tm_sec, trailinggarbage)
       != 7)
      || trailinggarbage[0] != '\0') {
    return 0;
  }

  // range for tm_mon is defined as 0-11
  --datetime.tm_mon;
  // tm_year is years since 1900
  datetime.tm_year -= 1900;

  // we don't know these, so we initialize to sane defaults
  datetime.tm_wday = datetime.tm_yday = 0;

  // set to -1 for "I don't know"
  datetime.tm_isdst = -1;

  time = mktime(&datetime);
  return time;
}

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

    template <typename F> void list_clients(F&& f, const char* outer)
    {
      if (!db) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Something went wrong (no db is set).\n");
      }

      if (!cap->list_clients(db, c_callback<F>, &f, outer)) {
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

  database OpenDb(const bareos::config::CatalogId& ctlg)
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

  template <typename Iter>
  std::string ListClientRestrictionStr(Iter filters_begin, Iter filters_end)
  {
    std::string res{};
    for (Iter current = filters_begin; current != filters_end; ++current) {
      if (current->has_name()) {
        auto& name = current->name();

        if (!res.empty()) { res += " AND "; }

        res += "name = '";
        res += name.match();
        res += "'";
      } else if (current->has_uname()) {
        auto& uname = current->uname();

        if (!res.empty()) { res += " AND "; }

        res += "uname = '";
        res += uname.match();
        res += "'";
      } else {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "illegal client filter");
      }
    }
    return res;
  }

  Status ListClients(ServerContext*,
                     const ListClientsRequest* request,
                     ListClientsResponse* response) override
  {
    try {
      if (!request->has_catalog()) {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "catalog is missing.");
      }
      auto db = OpenDb(request->catalog());

      auto range = request->options().range();
      auto filters = request->filters();

      auto restrictions
          = ListClientRestrictionStr(std::begin(filters), std::end(filters));

      auto offset = range.offset();
      auto limit = range.limit();

      if (limit == 0) {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "cannot query with limit 0.");
      }

      auto view = " LIMIT " + std::to_string(limit) + " OFFSET "
                  + std::to_string(offset);

      std::string query = "SELECT * FROM (%s) AS T";

      if (restrictions.size() > 0) {
        query += " WHERE ";
        query += restrictions;
      }

      query += view;

      db.list_clients(
          [writer = response->mutable_clients()](
              size_t field_count, const char* const* fields,
              const char* const* cols) -> bool {
            Client c;
            for (size_t i = 0; i < field_count; ++i) {
              if (strcmp(fields[i], "name") == 0) {
                c.set_name(cols[i]);
              } else if (strcmp(fields[i], "uname") == 0) {
                c.set_uname(cols[i]);
              } else if (strcmp(fields[i], "clientid") == 0) {
                c.mutable_id()->set_id(std::atoi(cols[i]));
              } else if (strcmp(fields[i], "autoprune") == 0) {
                c.set_autoprune(std::atoi(cols[i]) > 0);
              } else if (strcmp(fields[i], "fileretention") == 0) {
                c.set_fileretention(std::atoi(cols[i]));
              } else if (strcmp(fields[i], "jobretention") == 0) {
                c.set_jobretention(std::atoi(cols[i]));
              }
            }
            writer->Add(std::move(c));
            return true;
          },
          query.c_str());
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
      if (!request->has_catalog()) {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "catalog is missing.");
      }

      auto db = OpenDb(request->catalog());

      db.list_jobs([writer = response->mutable_jobs()](
                       size_t field_count, const char* const* fields,
                       const char* const* cols) -> bool {
        Job job;

        std::optional<time_t> start_time;
        std::optional<time_t> duration;

        for (size_t i = 0; i < field_count; ++i) {
          if (strcasecmp(fields[i], "jobid") == 0) {
            // job->set_jobid(std::atoi(cols[i]));
          } else if (strcasecmp(fields[i], "starttime") == 0) {
            if (auto secs_from_epoch = StrToUtime(cols[i]);
                secs_from_epoch >= 0) {
              start_time.emplace(secs_from_epoch);
              auto* start = job.mutable_start_time();
              start->set_seconds(secs_from_epoch);
              start->set_nanos(0);
            }
          } else if (strcasecmp(fields[i], "duration") == 0) {
            if (auto secs_from_epoch = StrToUtime(cols[i]);
                secs_from_epoch >= 0) {
              duration.emplace(secs_from_epoch);
            }
          } else if (strcasecmp(fields[i], "name") == 0) {
            job.set_name(cols[i]);
          } else if (strcasecmp(fields[i], "client") == 0) {
            // resp.mutable_client()->set_name(cols[i]);
          }
        }

        if (start_time && duration) {
          auto* end = job.mutable_end_time();
          end->set_seconds(*start_time + *duration);
          end->set_nanos(0);
        }

        writer->Add(std::move(job));
        return true;
      });
    } catch (const grpc_error& err) {
      return err.status;
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
