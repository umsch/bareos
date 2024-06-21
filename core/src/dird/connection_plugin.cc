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

#include "cats/sql_pooling.h"
#include "connection_plugin.h"
#include "connection_plugin/plugin.h"
#include "dird/create.h"
#include "dird_conf.h"
#include "dird_globals.h"
#include "job.h"
#include "jcr_util.h"
#include "include/baconfig.h"
#include "dird/restore.h"

#include <future>
#include <memory>
#include <optional>
#include <fstream>
#include <string_view>
#include <thread>
#include <dlfcn.h>

#include "lib/fnmatch.h"
#include "lib/parse_conf.h"
#include "connection_plugin/restore.h"
#include "connection_plugin/config.h"
#include "connection_plugin/database.h"
#include "lib/tree.h"

struct select_start_state {};

struct select_tree_state {
  TREE_ROOT* root{nullptr};
  size_t count{0};
  TREE_NODE* current{nullptr};

  std::string path;

  select_tree_state(TREE_ROOT* tree, size_t tree_count)
      : root{tree}, count{tree_count}, current{(TREE_NODE*)tree}
  {
  }

  ~select_tree_state() { FreeTree(root); }
};

struct build_stats {
  std::atomic<std::size_t> file_count;
  std::string error;

  std::size_t progress() { return file_count.load(); }
};

static directordaemon::InsertTreeContext build_tree(
    BareosDb* db,
    directordaemon::TreeArgs args,
    std::shared_ptr<build_stats>)
{
  auto ctx = directordaemon::BuildDirectoryTree(db, std::move(args));
  return ctx;
}

struct build_restore_tree_state {
  std::size_t expected_count;
  std::future<directordaemon::InsertTreeContext> ctx;
  std::shared_ptr<build_stats> stats;

  build_restore_tree_state(std::size_t count,
                           BareosDb* db,
                           directordaemon::TreeArgs args)
      : expected_count(count), stats{std::make_shared<build_stats>()}
  {
    ctx = std::async(std::launch::async, build_tree, db, std::move(args),
                     stats);
  }

  template <typename Duration> bool wait(Duration waittime)
  {
    ASSERT(ctx.valid());  // check the value was not yet collected

    return ctx.wait_for(waittime) == std::future_status::ready;
  }

  bool finished() { return wait(std::chrono::milliseconds(5)); }

  directordaemon::InsertTreeContext wait_and_get()
  {
    ASSERT(ctx.valid());
    return ctx.get();
  }
};

struct select_restore_option_state {
  directordaemon::JobResource* job{nullptr};
  directordaemon::ClientResource* restore_client{nullptr};
  directordaemon::CatalogResource* catalog{nullptr};

  std::string bsr;
  bool unlink_bsr;
  size_t count;

  ~select_restore_option_state()
  {
    if (bsr.size()) { ::unlink(bsr.c_str()); }
  }
};

struct job_started_state {
  JobId_t jobid;
};

struct restore_session_handle {
  std::string error;
  JobControlRecord* jcr;
  BareosDb* db;
  directordaemon::RestoreOptions opts;

  std::variant<select_start_state,
               build_restore_tree_state,
               select_tree_state,
               select_restore_option_state,
               job_started_state>
      state;


  ~restore_session_handle()
  {
    if (db) { DbSqlClosePooledConnection(jcr, db); }
    if (jcr) { FreeJcr(jcr); }
  }
};

struct database_session {
  std::optional<std::string> error;
  BareosDb* ptr{nullptr};

  ~database_session()
  {
    if (ptr) { DbSqlClosePooledConnection(nullptr, ptr); }
  }
};

namespace directordaemon {
void Log(log_severity severity, const char* str)
{
  Dmsg2(500, "%d: %s\n", severity, str);
}

static BareosDb* OpenDb(JobControlRecord* jcr)
{
  ResLocker _{my_config};
  auto* catalog = reinterpret_cast<CatalogResource*>(
      my_config->GetNextRes(R_CATALOG, NULL));
  if (!catalog) { return nullptr; }
  return DbSqlGetPooledConnection(
      jcr, catalog->db_driver, catalog->db_name, catalog->db_user,
      catalog->db_password.value, catalog->db_address, catalog->db_port,
      catalog->db_socket,
      /* mult_db_conn */ false, catalog->disable_batch_insert,
      catalog->try_reconnect, catalog->exit_on_fatal,
      /* private */ true);
}


restore_session_handle* PluginCreateRestoreSession(void)
{
  auto* jcr = NewDirectorJcr(DirdFreeJcr);
  if (!jcr) { return nullptr; }
  auto* db = OpenDb(jcr);
  if (!db) {
    FreeJcr(jcr);
    return nullptr;
  }
  auto* handle = new restore_session_handle{};

  handle->jcr = jcr;
  handle->db = db;

  return handle;
}

const char* PluginErrorString(restore_session_handle* handle)
{
  return handle->error.c_str();
}

bool PluginStartFromJobIds(restore_session_handle* handle,
                           size_t count,
                           const int64_t jobids[],
                           bool select_parents)
{
  auto* state = std::get_if<select_start_state>(&handle->state);
  if (!state) { return false; }

  if (select_parents) {
    handle->error = "Not Implemented Yet";
    return false;
  }

  TreeArgs args;
  args.initial_selection = TreeArgs::selection::None;
  for (size_t i = 0; i < count; ++i) { args.jobids.insert(jobids[i]); }
  args.estimated_size = 500;  // TODO: fix this

  auto& build_state = handle->state.emplace<build_restore_tree_state>(
      build_restore_tree_state(static_cast<std::size_t>(args.estimated_size),
                               handle->db, std::move(args)));

  auto ctx = build_state.wait_and_get();
  if (ctx.error) {
    handle->error = "Tree creation error: ERR=" + ctx.error.value();
    return false;
  }

  auto* root = ctx.root;
  auto size = ctx.TotalCount;
  handle->state.emplace<select_tree_state>(root, size);
  ctx.release();

  return true;
}

bool WriteFile(const char* name, std::string_view content)
{
  try {
    std::ofstream file(name);
    file << content;
    file.close();
    return true;
  } catch (...) {
    return false;
  }
}

bool PluginFinishSelection(restore_session_handle* handle,
                           const char* bootstrap)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  TREE_ROOT* root = state->root;

  std::unique_ptr bsr = BsrFromTree(root);

  if (!bsr) {
    handle->error = "No files selected to be restored.";
    return false;
  }

  std::optional error
      = AddVolumeInformationToBsr(handle->db, handle->jcr, bsr.get());
  if (error) {
    handle->error = "Could not finalize bsr: ERR=" + error.value();
    return false;
  }

  auto serialized = SerializeBsr(bsr.get());

  std::string bsr_path{};
  if (bootstrap) {
    bsr_path = bootstrap;
  } else {
    bsr_path = MakeUniqueBootstrapPath();
    bootstrap = bsr_path.c_str();
  }

  if (!WriteFile(bsr_path.c_str(), serialized.serialized)) {
    handle->error = "Could not write bootstrap to file: " + bsr_path;
    return false;
  }

  auto& newstate = handle->state.emplace<select_restore_option_state>();
  newstate.bsr = std::move(bsr_path);
  newstate.count = serialized.expected_count;
  newstate.unlink_bsr = (bootstrap == nullptr);

  return true;
}

const char* PluginGetBootstrapPath(restore_session_handle* handle)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return nullptr;
  }

  return state->bsr.c_str();
}

bool PluginCommitRestoreSession(restore_session_handle* handle,
                                job_started_info* info)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  if (!state->job) {
    handle->error = "No job selected";
    return false;
  }
  if (!state->restore_client) {
    handle->error = "No restore client selected";
    return false;
  }
  if (!state->catalog) {
    handle->error = "No catalog selected";
    return false;
  }

  RestoreOptions opts;

  opts.data = RestoreOptions::native_data{
      .BootStrapPath = state->bsr,
      .expected_file_count = static_cast<uint32_t>(state->count),
      .unlink_bsr = state->unlink_bsr,
  };
  opts.job = state->job;
  opts.restore_client = state->restore_client;
  opts.catalog = state->catalog;


  auto* jcr = CreateJob(std::move(opts));

  if (!jcr) {
    handle->error = "Could not create jcr";
    return false;
  }

  auto jobid = RunJob(jcr);

  if (jobid <= 0) {
    handle->error = "Could not create jcr";
    return false;
  }

  info->jobid = jobid;

  state->bsr.clear();

  handle->state = job_started_state{jobid};

  return true;
}

bool PluginCreateRestoreJob(restore_session_handle* handle,
                            restore_options Options,
                            job_started_info* info)
{
  (void)Options;
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  if (!state->job) {
    handle->error = "No job selected";
    return false;
  }
  if (!state->restore_client) {
    handle->error = "No restore client selected";
    return false;
  }
  if (!state->catalog) {
    handle->error = "No catalog selected";
    return false;
  }

  RestoreOptions opts;

  opts.data = RestoreOptions::native_data{
      .BootStrapPath = state->bsr,
      .expected_file_count = static_cast<uint32_t>(state->count),
      .unlink_bsr = state->unlink_bsr,
  };
  opts.job = state->job;
  opts.restore_client = state->restore_client;
  opts.catalog = state->catalog;


  auto* jcr = CreateJob(std::move(opts));

  if (!jcr) {
    handle->error = "Could not create jcr";
    return false;
  }

  auto jobid = RunJob(jcr);

  if (jobid <= 0) {
    handle->error = "Could not create jcr";
    return false;
  }

  info->jobid = jobid;

  state->bsr.clear();

  handle->state = job_started_state{jobid};

  return true;
}

bool PluginListFiles(restore_session_handle* handle,
                     file_callback* cb,
                     void* user)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  TREE_NODE* child = nullptr;

  while ((child = (TREE_NODE*)state->current->child.next(child))) {
    file_status status = {
        .name = child->fname,
        .marked = child->extract || child->extract_dir,
    };
    if (!(*cb)(user, status)) {
      handle->error = "user error";
      return false;
    }
  }

  return true;
}

bool PluginChangeDirectory(restore_session_handle* handle, const char* dir)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  std::string path{dir};
  auto* node = tree_cwd(path.data(), state->root, state->current);

  if (!node) {
    handle->error = "Could not change dir";
    return false;
  }

  state->current = node;

  return true;
}

const char* PluginCurrentDirectory(restore_session_handle* handle)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return nullptr;
  }

  {
    POOLMEM* path = tree_getpath(state->current);

    if (!path) {
      handle->error = "Internal error";
      return nullptr;
    }

    state->path.assign(path);

    FreeMemory(path);
  }

  return state->path.c_str();
}

bool PluginSetRestoreClient(restore_session_handle* handle,
                            const char* clientname)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Handle in wrong state.";
    return false;
  }
  bool lock = false;

  auto* client
      = (ClientResource*)my_config->GetResWithName(R_CLIENT, clientname, lock);
  if (!client) {
    handle->error = std::string{"Client not found: "} + clientname;
    return false;
  }

  state->restore_client = client;
  return true;
}
bool PluginSetRestoreJob(restore_session_handle* handle, const char* jobname)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Handle in wrong state.";
    return false;
  }
  bool lock = false;
  auto* job = (JobResource*)my_config->GetResWithName(R_JOB, jobname, lock);
  if (!job) {
    handle->error = std::string{"Job not found: "} + jobname;
    return false;
  }

  state->job = job;
  return true;
}
bool PluginSetCatalog(restore_session_handle* handle, const char* catalogname)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Handle in wrong state.";
    return false;
  }
  bool lock = false;
  auto* catalog = (CatalogResource*)my_config->GetResWithName(
      R_CATALOG, catalogname, lock);
  if (!catalog) {
    handle->error = std::string{"Catalog not found: "} + catalogname;
    return false;
  }

  state->catalog = catalog;
  return true;
}
bool PluginEnumerateOptions(restore_session_handle* handle,
                            key_value_handler* handler,
                            void* user)
{
  auto* state = std::get_if<select_restore_option_state>(&handle->state);
  if (!state) {
    handle->error = "Handle in wrong state.";
    return false;
  }

  if (!(*handler)(user, "job",
                  state->job ? state->job->resource_name_ : "UNSET")) {
    return false;
  }
  if (!(*handler)(user, "client",
                  state->restore_client ? state->restore_client->resource_name_
                                        : "UNSET")) {
    return false;
  }
  if (!(*handler)(user, "catalog",
                  state->catalog ? state->catalog->resource_name_ : "UNSET")) {
    return false;
  }
  if (!(*handler)(user, "bsr", state->bsr.c_str())) { return false; }

  return true;
}

void PluginAbortRestoreSession(restore_session_handle* handle)
{
  delete handle;
}

void PluginFinishRestoreSession(restore_session_handle* handle)
{
  delete handle;
}

static std::pair<std::string, std::string> split_path_into_dir_file(
    std::string_view path)
{
  auto pos = path.find_last_of('/');
  if (pos == path.npos) {
    return std::make_pair(std::string{}, std::string{path});
  } else if (pos == path.size()) {
    return std::make_pair(std::string{path}, std::string{});
  }

  return std::make_pair(std::string{path.substr(0, pos + 1)},
                        std::string{path.substr(pos + 1)});
}

bool PluginMarkUnmark(restore_session_handle* handle,
                      const char* regex,
                      bool mark,
                      file_callback* cb,
                      void* user)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  auto [dir, file] = split_path_into_dir_file(regex);

  if (dir.size()) {
    handle->error = "Unimplemented";
    return false;
  }

  TREE_NODE* child = nullptr;

  while ((child = (TREE_NODE*)state->current->child.next(child))) {
    if (child->extract != mark) {
      if (!fnmatch(file.c_str(), child->fname, 0)) { continue; }
      child->extract = mark;
      file_status status = {
          .name = child->fname,
          .marked = mark,
      };
      if (!(*cb)(user, status)) {
        handle->error = "user error";
        return false;
      }
    }
  }

  return true;
}

bool PluginConfigListClients(config_client_callback* cb, void* user)
{
  ResLocker _{my_config};

  ClientResource* client;
  foreach_res (client, R_CLIENT) {
    // making a defensive copy here to ensure that a bad plugin does not
    // corrupt the configuration
    std::string name{client->resource_name_};
    std::string address{client->address};
    bareos_config_client data = {
        .name = name.c_str(),
        .address = address.c_str(),
    };

    if (!cb(user, &data)) { return false; }
  }
  return true;
}

bool PluginConfigListJobs(config_job_callback* cb, void* user)
{
  ResLocker _{my_config};

  JobResource* job;
  foreach_res (job, R_JOB) {
    // making a defensive copy here to ensure that a bad plugin does not
    // corrupt the configuration
    std::string name{job->resource_name_};

    bareos_config_job data = {
        .name = name.c_str(),
        .type = static_cast<bareos_job_type>(job->JobType),
        .level = static_cast<bareos_job_level>(job->JobLevel),
    };
    if (!cb(user, &data)) { return false; }
  }
  return true;
}

bool PluginConfigListCatalogs(config_catalog_callback* cb, void* user)
{
  ResLocker _{my_config};

  CatalogResource* catalog;
  foreach_res (catalog, R_CATALOG) {
    // making a defensive copy here to ensure that a bad plugin does not
    // corrupt the configuration
    std::string name{catalog->resource_name_ ? catalog->resource_name_
                                             : "<UNSET>"};
    std::string db_name{catalog->db_name ? catalog->db_name : "<UNSET>"};

    bareos_config_catalog data = {
        .name = name.c_str(),
        .db_name = db_name.c_str(),
    };
    if (!cb(user, &data)) { return false; }
  }
  return true;
}

namespace database {

BareosDb* DB_Open(CatalogResource* res)
{
  return DbSqlGetPooledConnection(
      nullptr, res->db_driver, res->db_name, res->db_user,
      res->db_password.value, res->db_address, res->db_port, res->db_socket,
      res->mult_db_connections, res->disable_batch_insert, res->try_reconnect,
      res->exit_on_fatal, true);
}

database_session* OpenDatabase(const char* catalog_name)
{
  ResLocker _{my_config};

  auto* session = new database_session{};

  CatalogResource* catalog;
  foreach_res (catalog, R_CATALOG) {
    // making a defensive copy here to ensure that a bad plugin does not
    // corrupt the configuration
    if (strcmp(catalog->resource_name_, catalog_name) == 0) {
      session->ptr = DB_Open(catalog);
      if (!session->ptr) {
        session->error.emplace("Could not open db of catalog ");
        session->error.value() += catalog_name;
      }
      return session;
    }
  }
  session->error.emplace("No catalog found with name ");
  session->error.value() += catalog_name;

  return session;
}

void CloseDatabase(database_session* sess) { delete sess; }

struct sql_result_handler : public list_result_handler {
  sql_result_handler(DB_result_callback* cb_, void* user_)
      : cb(cb_), user(user_)
  {
  }

  void begin(const char* name_) override { name = name_; }
  void add_field(SQL_FIELD* field, field_flags) override
  {
    fields.emplace_back(strdup(field->name));
  }

  bool handle(SQL_ROW row) override
  {
    return cb(user, fields.size(), fields.data(), row);
  }

  void end() override {}


  ~sql_result_handler()
  {
    for (auto* field : fields) { free(field); }
  }

  std::string name;
  DB_result_callback* cb;
  void* user;

  std::vector<char*> fields;
};

bool ListClients(database_session* sess, DB_result_callback* cb, void* user)
{
  if (!sess) { return false; }

  sess->error.reset();

  sql_result_handler handler(cb, user);

  return sess->ptr->ListClientRecords(nullptr, nullptr, false, &handler);
}

bool ListJobs(database_session* sess, DB_result_callback* cb, void* user)
{
  if (!sess) { return false; }

  sess->error.reset();

  sql_result_handler handler(cb, user);

  (void)cb;
  (void)user;

  return false;
  // return sess->ptr->ListJobRecords(nullptr, nullptr, false, &handler);
}

const char* ErrorString(database_session* sess)
{
  if (sess->error) { return sess->error->c_str(); }
  return nullptr;
}
};  // namespace database

template <typename T> bool check_buffer(size_t bufsize, void* buffer)
{
  if (bufsize != sizeof(T)) { return false; }
  if (!std::align(alignof(T), bufsize, buffer, bufsize)) { return false; }
  return true;
}

bool QueryCabability(bareos_capability Cap, size_t bufsize, void* buffer)
{
  switch (Cap) {
    case CAP_Restore: {
      if (check_buffer<restore_capability>(bufsize, buffer)) {
        restore_capability cap = {
            .list_files = &PluginListFiles,
            .change_directory = &PluginChangeDirectory,
            .mark_unmark = &PluginMarkUnmark,
            .error_string = &PluginErrorString,
            .current_directory = &PluginCurrentDirectory,
            .create_restore_session = &PluginCreateRestoreSession,
            .start_from_jobids = &PluginStartFromJobIds,
            .finish_restore_session = &PluginFinishRestoreSession,
            .create_restore_job = &PluginCreateRestoreJob,
        };
        memcpy(buffer, &cap, sizeof(cap));
        return true;
      }
    } break;
    case CAP_Config: {
      if (check_buffer<config_capability>(bufsize, buffer)) {
        config_capability cap = {
            .list_clients = &PluginConfigListClients,
            .list_jobs = &PluginConfigListJobs,
            .list_catalogs = &PluginConfigListCatalogs,
        };
        memcpy(buffer, &cap, sizeof(cap));
        return true;
      }
    } break;
    case CAP_Database: {
      if (check_buffer<database_capability>(bufsize, buffer)) {
        database_capability cap = {
            .open_database = &database::OpenDatabase,
            .close_database = &database::CloseDatabase,
            .list_clients = &database::ListClients,
            .list_jobs = &database::ListJobs,
            .error_string = &database::ErrorString,
        };
        memcpy(buffer, &cap, sizeof(cap));
        return true;
      }
    } break;
  }

  return false;
}

struct conn_plugin {
  std::string name;
  plugin_api api;
  void* handle;
};

std::vector<conn_plugin>* loaded = nullptr;

static std::optional<conn_plugin> LoadPlugin(const char* directory,
                                             const char* name)
{
  auto path = std::string{directory};
  if (path.back() != '/') { path += '/'; }
  path += "lib";
  path += name;
  path += ".so";

  void* handle = dlopen(path.c_str(), RTLD_NOW);

  if (!handle) { return std::nullopt; }

#define ASSTR_IMPL(x) #x
#define ASSTR(x) ASSTR_IMPL(x)
  const char* symbol = ASSTR(CONN_PLUGIN_API_SYMBOL_NAME);
#undef ASSTR
#undef ASSTR_IMPL

  auto api = reinterpret_cast<const plugin_api*>(dlsym(handle, symbol));

  if (!api) { return std::nullopt; }

  if (api->size != sizeof(*api)) { return std::nullopt; }

  return conn_plugin{name, *api, handle};
}

bool LoadConnectionPlugins(const char* directory,
                           const std::vector<std::string>& names)
{
  ASSERT(!loaded);

  std::vector<conn_plugin> plugins;

  for (auto& name : names) {
    if (auto plugin = LoadPlugin(directory, name.c_str())) {
      plugins.emplace_back(std::move(plugin).value());
    } else {
      return false;
    }
  }

  bareos_api api = {
      .size = sizeof(bareos_api),
      .log = &Log,
      .query = &QueryCabability,
  };

  ResLocker _{my_config};


  auto* p = dynamic_cast<GrpcResource*>(my_config->GetNextRes(R_GRPC, nullptr));

  if (!p) { return false; }

  if (p->addrs->size() == 0) {
    Emsg0(M_ERROR, 0, "No address specified for connection plugins.\n");
    return false;
  } else if (p->addrs->size() > 1) {
    Emsg0(M_WARNING, 0,
          "More than one address specified for connection plugins; all but the "
          "first are ignored.\n");
  }

  auto* addr = p->addrs->first();

  // TODO: use full address instead
  auto port = addr->GetPortHostOrder();

  for (auto& plugin : plugins) {
    if (!plugin.api.load(&api)) { return false; }
    if (!plugin.api.start(port)) { return false; }
  }

  loaded = new std::vector<conn_plugin>(std::move(plugins));

  return true;
}

void UnloadConnectionPlugins()
{
  ASSERT(loaded);

  for (auto& plugin : *loaded) { plugin.api.unload(); }
}
}  // namespace directordaemon
