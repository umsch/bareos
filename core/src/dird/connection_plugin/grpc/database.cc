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
#include "grpc.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

namespace {

using Int = std::int64_t;
using Text = const char*;

enum class enum_option
{
  Required,
  Optional
};

template <typename U> struct definition {
  definition() = default;
  template <typename T>
  constexpr definition(std::string_view def_name,
                       T U::*member,
                       enum_option opt = enum_option::Required,
                       const char* defval = nullptr)
      : name{def_name}, offset{member}, option{opt}, default_value{defval}
  {
  }

  std::string_view name{};
  std::variant<Int U::*,
               Text U::*,
               std::optional<Int> U::*,
               std::optional<Text> U::*>
      offset{};
  enum_option option{enum_option::Required};
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

template <typename T> struct definition_of;

template <typename T> struct builder {
  constexpr static auto def = definition_of<T>::def;

  template <typename Inner, typename Test>
  static constexpr bool has_type
      = std::is_same_v<Inner, Test T::*>
        || std::is_same_v<Inner, std::optional<Test> T::*>;


  template <typename U> constexpr static size_t type_count()
  {
    size_t count = 0;

    for (size_t i = 0; i < def.size(); ++i) {
      std::visit(
          [&count](auto&& val) {
            using Inner = std::decay_t<decltype(val)>;
            if constexpr (has_type<Inner, U>) {
              count += 1;
            } else {
            }
          },
          def[i].offset);
    }

    return count;
  }

  constexpr static auto get_conversions()
  {
    std::array<size_t, def.size()> result{};

    size_t current_int = 0;
    size_t current_text = 0;

    for (size_t i = 0; i < def.size(); ++i) {
      std::visit(
          [cur = &result[i], &current_int, &current_text](auto&& val) {
            using Inner = std::decay_t<decltype(val)>;
            if constexpr (has_type<Inner, Int>) {
              *cur = current_int++;
            } else if constexpr (has_type<Inner, Text>) {
              *cur = current_text++;
            } else {
              static_assert(0, "unhandled type");
            }
          },
          def[i].offset);
    }

    return result;
  }

  constexpr static auto conversions = get_conversions();

  template <typename Inner>
  using counted_array = std::array<std::optional<Inner>, type_count<Inner>()>;

  counted_array<Int> ints;
  counted_array<Text> texts;

  using Value = std::variant<std::optional<Int>*, std::optional<Text>*>;


  Value get(const definition<T>& member)
  {
    auto index = std::distance(&def.front(), &member);
    auto sub_index = conversions[index];
    return std::visit(
        [this, sub_index](auto&& val) -> Value {
          using Inner = std::decay_t<decltype(val)>;
          if constexpr (has_type<Inner, Text>) {
            return &texts[sub_index];
          } else if constexpr (has_type<Inner, Int>) {
            return &ints[sub_index];
          } else {
            static_assert(0, "unhanled type");
          }
        },
        member.offset);
  }


  bool set(std::string_view key, const char* value)
  {
    for (auto& member : def) {
      if (key == member.name) {
        return std::visit(
            [value](auto&& val) {
              if (val->has_value()) { return false; }
              val->emplace();
              return set_value(val->value(), value);
            },
            get(member));
      }
    }

    return false;
  }

  std::optional<T> finalize()
  {
    if (!set_defaults()) { return std::nullopt; }

    T result;
    for (auto& member : def) {
      auto ok = std::visit(
          [&result](auto&& place, auto&& value) {
            using InnerP = std::decay_t<decltype(result.*place)>;
            using InnerV = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<InnerV, std::optional<Text>*>) {
              if constexpr (std::is_same_v<InnerP, Text>) {
                if (!value->has_value()) { return false; }
                result.*place = value->value();
                return true;
              } else if constexpr (std::is_same_v<InnerP,
                                                  std::optional<Text>>) {
                result.*place = *value;
                return true;
              } else {
                return false;
              }
            } else if constexpr (std::is_same_v<InnerV, std::optional<Int>*>) {
              if constexpr (std::is_same_v<InnerP, Int>) {
                if (!value->has_value()) { return false; }
                result.*place = value->value();
                return true;
              } else if constexpr (std::is_same_v<InnerP, std::optional<Int>>) {
                result.*place = *value;
                return true;
              } else {
                return false;
              }
            } else {
              static_assert(0, "unhandled type");
            }
          },
          member.offset, get(member));

      if (!ok) { return std::nullopt; }
    }

    return result;
  }

 private:
  bool set_defaults()
  {
    for (auto& member : def) {
      if (!member.default_value) { continue; }

      auto ok = std::visit(
          [value = member.default_value](auto&& val) {
            if (val->has_value()) { return true; }
            return set_value(val->value(), value);
          },
          get(member));

      if (!ok) { return false; }
    }

    return true;
  }
};

template <typename T, size_t N>
constexpr std::array<T, N> make_array(const T (&arr)[N])
{
  std::array<T, N> ret{};

  for (size_t i = 0; i < N; ++i) { ret[i] = arr[i]; }

  return ret;
}

struct job_db_entry {
  Int jobId;
  Text job;
  Text name;
  Text type;
  Text level;
  std::optional<Int> clientId;
  Text jobStatus;

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

  std::optional<bareos::database::Job> create() const;
};

template <> struct definition_of<job_db_entry> {
  constexpr static auto def = make_array<definition<job_db_entry>>({
      {"jobid", &job_db_entry::jobId},
      {"job", &job_db_entry::job},
      {"name", &job_db_entry::name},
      {"type", &job_db_entry::type},
      {"level", &job_db_entry::level},
      {"clientid", &job_db_entry::clientId},
      {"jobstatus", &job_db_entry::jobStatus},
      {"schedtime", &job_db_entry::schedTime, enum_option::Optional},
      {"starttime", &job_db_entry::startTime, enum_option::Optional},
      {"endtime", &job_db_entry::endTime, enum_option::Optional},
      {"realendtime", &job_db_entry::realEndTime, enum_option::Optional},
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
      {"comment", &job_db_entry::comment, enum_option::Optional},
  });
};

struct client_db_entry {
  Int clientId;
  Text name;
  Text uName;

  std::optional<Int> autoPrune;
  std::optional<Int> fileRetention;
  std::optional<Int> jobRetention;

  std::optional<bareos::database::Client> create() const noexcept
  {
    bareos::database::Client c;
    c.set_name(name);
    c.set_uname(uName);
    c.mutable_id()->set_id(clientId);

    if (autoPrune) { c.set_autoprune(autoPrune.value()); }
    if (jobRetention) { c.set_jobretention(jobRetention.value()); }
    if (fileRetention) { c.set_fileretention(fileRetention.value()); }

    return c;
  }
};

template <> struct definition_of<client_db_entry> {
  constexpr static auto def = make_array<definition<client_db_entry>>({
      {"clientid", &client_db_entry::clientId},
      {"name", &client_db_entry::name},
      {"uname", &client_db_entry::uName},
      {"autoprune", &client_db_entry::autoPrune},
      {"fileretention", &client_db_entry::fileRetention},
      {"jobretention", &client_db_entry::jobRetention},
  });
};

std::optional<bareos::database::JobType> job_type_from(std::string_view v)
{
  if (v.size() != 1) { return std::nullopt; }

  if (v[0] == 'B') { return bareos::database::JobType::BACKUP; }
  if (v[0] == 'M') { return bareos::database::JobType::MIGRATED_JOB; }
  if (v[0] == 'V') { return bareos::database::JobType::VERIFY; }
  if (v[0] == 'R') { return bareos::database::JobType::RESTORE; }
  if (v[0] == 'U') { return bareos::database::JobType::CONSOLE; }
  if (v[0] == 'I') { return bareos::database::JobType::SYSTEM; }
  if (v[0] == 'D') { return bareos::database::JobType::ADMIN; }
  if (v[0] == 'A') { return bareos::database::JobType::ARCHIVE; }
  if (v[0] == 'C') { return bareos::database::JobType::JOB_COPY; }
  if (v[0] == 'c') { return bareos::database::JobType::COPY; }
  if (v[0] == 'g') { return bareos::database::JobType::MIGRATE; }
  if (v[0] == 'S') { return bareos::database::JobType::SCAN; }
  if (v[0] == 'O') { return bareos::database::JobType::CONSOLIDATE; }

  return std::nullopt;
}

std::optional<bareos::database::JobLevel> job_level_from(std::string_view v)
{
  if (v.size() != 1) { return std::nullopt; }

  if (v[0] == 'F') { return bareos::database::JobLevel::FULL; }
  if (v[0] == 'I') { return bareos::database::JobLevel::INCREMENTAL; }
  if (v[0] == 'D') { return bareos::database::JobLevel::DIFFERENTIAL; }
  // if (v[0] == 'S') { return bareos::database::JobLevel::SINCE; }
  if (v[0] == 'C') { return bareos::database::JobLevel::VERIFY_CATALOG; }
  if (v[0] == 'V') { return bareos::database::JobLevel::VERIFY_INIT; }
  if (v[0] == 'O') {
    return bareos::database::JobLevel::VERIFY_VOLUME_TO_CATALOG;
  }
  if (v[0] == 'd') {
    return bareos::database::JobLevel::VERIFY_DISK_TO_CATALOG;
  }
  if (v[0] == 'A') { return bareos::database::JobLevel::VERIFY_DATA; }
  if (v[0] == 'B') { return bareos::database::JobLevel::BASE; }
  if (v[0] == ' ') { return bareos::database::JobLevel::NONE; }
  if (v[0] == 'f') { return bareos::database::JobLevel::VIRTUAL_FULL; }

  return std::nullopt;
}
};  // namespace

// Convert standard time string yyyy-mm-dd hh:mm:ss to Unix time
std::optional<time_t> MyStrToUtime(const char* str)
{
  tm datetime{};

  // Check for bad argument
  if (!str || *str == 0) { return std::nullopt; }

  if (sscanf(str, "%u-%u-%u %u:%u:%u", &datetime.tm_year, &datetime.tm_mon,
             &datetime.tm_mday, &datetime.tm_hour, &datetime.tm_min,
             &datetime.tm_sec)
      != 6) {
    return std::nullopt;
  }

  if (datetime.tm_mon == 0 || datetime.tm_year < 1900) { return std::nullopt; }

  // range for tm_mon is defined as 0-11
  --datetime.tm_mon;
  // tm_year is years since 1900
  datetime.tm_year -= 1900;

  // we don't know these, so we initialize to sane defaults
  datetime.tm_wday = datetime.tm_yday = 0;

  // set to -1 for "I don't know"
  datetime.tm_isdst = -1;

  time_t time = mktime(&datetime);
  if (time == (time_t)-1) { return std::nullopt; }
  return std::make_optional(time);
}

std::optional<bareos::database::Job> job_db_entry::create() const
{
  bareos::database::Job job;
  job.mutable_id()->set_id(jobId);
  job.set_name(name);
  if (auto t = job_type_from(type)) {
    job.set_type(*t);
  } else {
    return std::nullopt;
  }
  if (schedTime) {
    auto* ts = job.mutable_sched_time();
    auto seconds = MyStrToUtime(*schedTime);
    if (!seconds) { return std::nullopt; }
    ts->set_seconds(*seconds);
    ts->set_nanos(0);
  }
  if (startTime) {
    auto* ts = job.mutable_start_time();
    auto seconds = MyStrToUtime(*startTime);
    if (!seconds) { return std::nullopt; }
    ts->set_seconds(*seconds);
    ts->set_nanos(0);
  }
  if (endTime) {
    auto* ts = job.mutable_end_time();
    auto seconds = MyStrToUtime(*endTime);
    if (!seconds) { return std::nullopt; }
    ts->set_seconds(*seconds);
    ts->set_nanos(0);
  }
  job.set_comment(*comment);

  switch (job.type()) {
    case bareos::database::JobType::BACKUP: {
      if (!clientId || !jobFiles || !jobBytes) {
        return std::nullopt;
      } else {
        auto* data = job.mutable_backup();

        if (auto l = job_level_from(level)) {
          data->set_level(*l);
        } else {
          return std::nullopt;
        }
        data->mutable_client()->set_id(*clientId);
        data->set_job_files(*jobFiles);
        data->set_job_bytes(*jobBytes);
      }
    } break;
    default: {
    } break;
  }
  return job;
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

  Status ClientInfo(ServerContext*,
                    const ClientInfoRequest* request,
                    ClientInfoResponse* response) override
  {
    try {
      if (!request->has_catalog()) {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "catalog is missing.");
      }

      auto db = OpenDb(request->catalog());

      auto ids = request->ids();

      std::string s;

      for (int i = 0; i < ids.size(); ++i) {
        if (i != 0) { s += ","; }

        s += std::to_string(ids[i].id());
      }

      std::string query
          = "SELECT * FROM (%s) AS t WHERE clientid IN (" + s + ")";

      db.list_clients(
          [writer = response->mutable_clients()](
              size_t field_count, const char* const* fields,
              const char* const* cols) -> bool {
            builder<client_db_entry> builder{};
            for (size_t i = 0; i < field_count; ++i) {
              if (!builder.set(fields[i], cols[i])) {
                // ignore this client
                return true;
              }
            }

            std::optional entry = builder.finalize();
            if (!entry) {
              // ignore this client
              return true;
            }

            auto client = entry->create();

            if (!client) {
              // ignore this client
              return true;
            }

            writer->Add(std::move(client).value());
            return true;
          },
          query.c_str());

    } catch (const grpc_error& err) {
      return err.status;
    }

    return Status::OK;
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
            builder<client_db_entry> bob;

            for (size_t i = 0; i < field_count; ++i) {
              // skip bad table entries
              if (!bob.set(fields[i], cols[i])) { return true; }
            }

            std::optional entry = bob.finalize();

            // skip bad table entries
            if (!entry) { return true; }

            auto client = entry->create();

            if (!client) {
              // ignore this client
              return true;
            }

            writer->Add(std::move(client).value());
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

        res += "clientid = ";
        res += std::to_string(client.id().id());

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
          case bareos::database::JOB_TYPE_UNSPECIFIED:
          case bareos::database::JobType_INT_MIN_SENTINEL_DO_NOT_USE_:
          case bareos::database::JobType_INT_MAX_SENTINEL_DO_NOT_USE_: {
            throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                             "job type filter not set correctly");
          } break;
          case bareos::database::RESTORE: {
            res += "R";
          } break;
          case bareos::database::BACKUP: {
            res += "B";
          } break;
          case bareos::database::COPY: {
            res += "c";
          } break;
          case bareos::database::VERIFY: {
            res += "V";
          } break;
          case bareos::database::ADMIN: {
            res += "D";
          } break;
          case bareos::database::ARCHIVE: {
            res += "A";
          } break;
          case bareos::database::MIGRATE: {
            res += "g";
          } break;
          case bareos::database::CONSOLIDATE: {
            res += "O";
          } break;
          case bareos::database::MIGRATED_JOB: {
            res += "M";
          } break;
          case bareos::database::CONSOLE: {
            res += "U";
          } break;
          case bareos::database::SYSTEM: {
            res += "I";
          } break;
          case bareos::database::JOB_COPY: {
            res += "C";
          } break;
          case bareos::database::SCAN: {
            res += "S";
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

  Status JobInfo(ServerContext*,
                 const JobInfoRequest* request,
                 JobInfoResponse* response) override
  {
    try {
      if (!request->has_catalog()) {
        throw grpc_error(grpc::StatusCode::INVALID_ARGUMENT,
                         "catalog is missing.");
      }

      auto db = OpenDb(request->catalog());

      auto ids = request->ids();

      std::string s;

      for (int i = 0; i < ids.size(); ++i) {
        if (i != 0) { s += ","; }

        s += std::to_string(ids[i].id());
      }

      std::string query = "SELECT * FROM (%s) AS t WHERE jobid IN (" + s + ")";

      db.list_jobs(
          [writer = response->mutable_jobs()](size_t field_count,
                                              const char* const* fields,
                                              const char* const* cols) -> bool {
            builder<job_db_entry> builder{};
            for (size_t i = 0; i < field_count; ++i) {
              if (!builder.set(fields[i], cols[i])) {
                // ignore this job
                return true;
              }
            }

            std::optional entry = builder.finalize();
            if (!entry) {
              // ignore this job
              return true;
            }

            auto job = entry->create();

            if (!job) {
              // ignore this job
              return true;
            }

            writer->Add(std::move(job).value());
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
            builder<job_db_entry> builder{};
            for (size_t i = 0; i < field_count; ++i) {
              if (!builder.set(fields[i], cols[i])) {
                // ignore this job
                return true;
              }
            }

            std::optional entry = builder.finalize();
            if (!entry) {
              // ignore this job
              return true;
            }


            auto job = entry->create();

            if (!job) {
              // ignore this job
              return true;
            }

            writer->Add(std::move(job).value());
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
