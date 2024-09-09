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
#include <variant>
#include "common.pb.h"
#include "grpc.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace {

using Int = std::int64_t;
using Text = const char*;


template <typename T, typename U> uintptr_t calculate_offset(T U::*ptr)
{
  U* base = nullptr;

  T* member = &(base->*ptr);

  return reinterpret_cast<uintptr_t>(member);
}

struct definition;

struct defined_struct {
  virtual std::pair<size_t, const definition*> introspect() = 0;

  bool set(std::string_view key, const char* value);

  bool finalize();
};

template <typename T, typename U>
constexpr std::optional<T> defined_struct::*defined_offset_of(
    std::optional<T> U::*ptr)
{
  return static_cast<std::optional<T> defined_struct::*>(ptr);
}

struct definition {
  definition() = default;
  template <typename T, typename U>
  constexpr definition(std::string_view def_name,
                       std::optional<T> U::*member,
                       const char* defval = nullptr)
      : name{def_name}, offset{defined_offset_of(member)}, default_value{defval}
  {
  }

  std::string_view name{};
  std::variant<std::optional<Int> defined_struct::*,
               std::optional<Text> defined_struct::*>
      offset{};
  const char* default_value{};
};

template <typename T> bool set_value(T& place, const char* value) = delete;

template <> bool set_value(Int& place, const char* value)
{
  place = std::atoi(value);
  return true;
}

template <> bool set_value(Text& place, const char* value)
{
  place = value;
  return true;
}

bool defined_struct::set(std::string_view key, const char* value)
{
  auto [len, ptr] = introspect();

  for (size_t i = 0; i < len; ++i) {
    if (key == ptr[i].name) {
      std::visit(
          [this, value](auto&& val) {
            auto& opt = (this->*val);
            if (opt.has_value()) { return false; }
            return set_value(*opt, value);
          },
          ptr[i].offset);
    }
  }

  return false;
}

bool defined_struct::finalize()
{
  auto [len, ptr] = introspect();

  bool ok = true;

  for (size_t i = 0; i < len; ++i) {
    std::visit(
        [this, &ok, defval = ptr[i].default_value](auto&& val) {
          auto& opt = (this->*val);

          if (!opt.has_value()) {
            if (defval) {
              ok = set_value(*opt, defval);
            } else {
              ok = false;
            }
          }
        },
        ptr[i].offset);
  }

  return ok;
}


template <typename T, size_t N>
constexpr std::array<T, N> make_array(const T (&arr)[N])
{
  std::array<T, N> ret{};

  for (size_t i = 0; i < N; ++i) { ret[i] = arr[i]; }

  return ret;
}

struct job_db_entry : defined_struct {
  std::optional<Int> jobId;
  std::optional<Text> job;
  std::optional<Text> name;
  std::optional<Text> type;
  std::optional<Text> level;
  std::optional<Int> clientId;
  std::optional<Text> jobStatus;
  std::optional<Text> schedTime;
  std::optional<Text> startTime;
  std::optional<Text> endTime;
  std::optional<Text> realEndTime;
  std::optional<Int> jobTDate;
  std::optional<Int> volSessionId;
  std::optional<Int> volSessionTime;
  std::optional<Int> jobFiles;
  std::optional<Int> jobBytes;
  std::optional<Int> readBytes;
  std::optional<Int> jobErrors;
  std::optional<Int> jobMissingFiles;
  std::optional<Int> poolId;
  std::optional<Int> filesetId;
  std::optional<Int> priorJobid;
  std::optional<Int> purgedFiles;
  std::optional<Int> hasBase;
  std::optional<Int> hasCache;
  std::optional<Int> reviewed;
  std::optional<Text> comment;

  std::pair<size_t, const definition*> introspect() override
  {
    constexpr static auto job_def = make_array<definition>({
        {"jobid", &job_db_entry::jobId},
        {"job", &job_db_entry::job},
        {"name", &job_db_entry::name},
        {"type", &job_db_entry::type},
        {"level", &job_db_entry::level},
        {"clientid", &job_db_entry::clientId},
        {"jobstatus", &job_db_entry::jobStatus},
        {"schedtime", &job_db_entry::schedTime},
        {"starttime", &job_db_entry::startTime},
        {"endtime", &job_db_entry::endTime},
        {"realendtime", &job_db_entry::realEndTime},
        {"jobtdate", &job_db_entry::jobTDate},
        {"volsessionid", &job_db_entry::volSessionId},
        {"volsessiontime", &job_db_entry::volSessionTime},
        {"jobfiles", &job_db_entry::jobFiles},
        {"jobbytes", &job_db_entry::jobBytes},
        {"readbytes", &job_db_entry::readBytes},
        {"joberrors", &job_db_entry::jobErrors},
        {"jobmissingfiles", &job_db_entry::jobMissingFiles},
        {"poolid", &job_db_entry::poolId},
        {"filesetid", &job_db_entry::filesetId},
        {"priorjobid", &job_db_entry::priorJobid},
        {"purgedfiles", &job_db_entry::purgedFiles},
        {"hasbase", &job_db_entry::hasBase},
        {"hascache", &job_db_entry::hasCache},
        {"reviewed", &job_db_entry::reviewed},
        {"comment", &job_db_entry::comment},
    });

    return {job_def.size(), job_def.data()};
  }
};

bareos::common::JobType job_type_from(std::string_view v)
{
  if (v.size() != 1) {
    // throw error
  }

  if (v[0] == 'B') { return bareos::common::JobType::BACKUP; }
  // if (v[0] == 'M') { return bareos::common::JobType::MIGRATED_JOB; }
  if (v[0] == 'V') { return bareos::common::JobType::VERIFY; }
  if (v[0] == 'R') { return bareos::common::JobType::RESTORE; }
  // if (v[0] == 'U') { return bareos::common::JobType::CONSOLE; }
  // if (v[0] == 'I') { return bareos::common::JobType::SYSTEM; }
  if (v[0] == 'D') { return bareos::common::JobType::ADMIN; }
  if (v[0] == 'A') { return bareos::common::JobType::ARCHIVE; }
  // if (v[0] == 'C') { return bareos::common::JobType::JOB_COPY; }
  if (v[0] == 'c') { return bareos::common::JobType::COPY; }
  if (v[0] == 'g') { return bareos::common::JobType::MIGRATE; }
  // if (v[0] == 'S') { return bareos::common::JobType::SCAN; }
  if (v[0] == 'O') { return bareos::common::JobType::CONSOLIDATE; }

  // throw error
  return bareos::common::JobType::BACKUP;
}

bareos::common::JobLevel job_level_from(std::string_view v)
{
  if (v.size() != 1) {
    // throw error
  }
  if (v[0] == 'F') { return bareos::common::JobLevel::FULL; }
  if (v[0] == 'I') { return bareos::common::JobLevel::INCREMENTAL; }
  if (v[0] == 'D') { return bareos::common::JobLevel::DIFFERENTIAL; }
  // if (v[0] == 'S') { return bareos::common::JobLevel::SINCE; }
  // if (v[0] == 'C') { return bareos::common::JobLevel::VERIFY_CATALOG; }
  // if (v[0] == 'V') { return bareos::common::JobLevel::VERIFY_INIT; }
  // if (v[0] == 'O') { return
  // bareos::common::JobLevel::VERIFY_VOLUME_TO_CATALOG; } if (v[0] == 'd') {
  // return bareos::common::JobLevel::VERIFY_DISK_TO_CATALOG; } if (v[0] == 'A')
  // { return bareos::common::JobLevel::VERIFY_DATA; } if (v[0] == 'B') { return
  // bareos::common::JobLevel::BASE; } if (v[0] == ' ') { return
  // bareos::common::JobLevel::NONE; } if (v[0] == 'f') { return
  // bareos::common::JobLevel::VIRTUAL_FULL; }

  // throw error
  return bareos::common::JobLevel::FULL;
}
};  // namespace

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

    template <typename F> void list_jobs(F&& f, const char* outer)
    {
      if (!db) {
        throw grpc_error(grpc::StatusCode::UNKNOWN,
                         "Something went wrong (no db is set).\n");
      }

      if (!cap->list_jobs(db, c_callback<F>, &f, outer)) {
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

  template <typename Iter>
  std::string ListJobRestrictionStr(Iter filters_begin, Iter filters_end)
  {
    std::string res{};
    for (Iter current = filters_begin; current != filters_end; ++current) {
      if (current->has_client()) {
        auto& client = current->client();

        if (!res.empty()) { res += " AND "; }

        res += "clientid = '";
        res += client.id().id();
        res += "'";

      } else if (current->has_name()) {
        auto& name = current->name();

        if (!res.empty()) { res += " AND "; }

        res += "name = '";
        res += name.match();
        res += "'";
      } else if (current->has_type()) {
        auto& type = current->type();

        if (!res.empty()) { res += " AND "; }


        res += "type = '";
        switch (type.type()) {
          case common::JOB_TYPE_UNSPECIFIED:
          case common::JobType_INT_MIN_SENTINEL_DO_NOT_USE_:
          case common::JobType_INT_MAX_SENTINEL_DO_NOT_USE_: {
            throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                             "job type filter not set correctly");
          } break;
          case common::RESTORE: {
            res += "R";
          } break;
          case common::BACKUP: {
            res += "B";
          } break;
          case common::COPY: {
            res += "c";
          } break;
          case common::VERIFY: {
            res += "V";
          } break;
          case common::ADMIN: {
            res += "D";
          } break;
          case common::ARCHIVE: {
            res += "A";
          } break;
          case common::MIGRATE: {
            res += "g";
          } break;
          case common::CONSOLIDATE: {
            res += "O";
          } break;
        }
        res += "'";

      } else {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "illegal job filter");
      }
    }
    return res;
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

      auto range = request->options().range();
      auto filters = request->filters();

      auto restrictions
          = ListJobRestrictionStr(std::begin(filters), std::end(filters));

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

      db.list_jobs(
          [writer = response->mutable_jobs()](size_t field_count,
                                              const char* const* fields,
                                              const char* const* cols) -> bool {
            job_db_entry ent{};

            for (size_t i = 0; i < field_count; ++i) {
              if (!ent.set(fields[i], cols[i])) {
                // throw error
              }
            }

            if (!ent.finalize()) {
              // throw error
            }


            Job job;
            job.mutable_id()->set_id(*ent.jobId);
            job.set_name(*ent.name);
            job.set_type(job_type_from(*ent.type));
            {
              auto* ts = job.mutable_sched_time();
              time_t t = StrToUtime(*ent.schedTime);
              ts->set_seconds(t);
              ts->set_nanos(0);
            }
            {
              auto* ts = job.mutable_start_time();
              time_t t = StrToUtime(*ent.schedTime);
              ts->set_seconds(t);
              ts->set_nanos(0);
            }
            {
              auto* ts = job.mutable_end_time();
              time_t t = StrToUtime(*ent.schedTime);
              ts->set_seconds(t);
              ts->set_nanos(0);
            }
            job.set_comment(*ent.comment);

            switch (job.type()) {
              case common::JobType::BACKUP: {
                auto* data = job.mutable_backup();

                data->set_level(job_level_from(*ent.level));
                data->mutable_client()->set_id(*ent.clientId);
                data->set_job_files(*ent.jobFiles);
                data->set_job_bytes(*ent.jobBytes);

              } break;
              default: {
              } break;
            }

            writer->Add(std::move(job));
            return true;
          },
          query.c_str());
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
