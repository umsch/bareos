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
#include "include/filetypes.h"
#include "include/protocol_types.h"
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
#include <jansson.h>

#include "lib/fnmatch.h"
#include "lib/output_formatter_resource.h"
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
  std::vector<uint64_t> selected_jobids{};

  // immutable options
  std::string jobids;

  // mutable options
  std::optional<directordaemon::ClientResource*> restore_client{};
  directordaemon::JobResource* restore_job{};
  std::optional<std::string> restore_location{};
  std::optional<replace_option> replace{};

  select_tree_state(TREE_ROOT* tree,
                    size_t tree_count,
                    std::string&& jobids_,
                    directordaemon::JobResource* restore_job)
      : root{tree}
      , count{tree_count}
      , current{(TREE_NODE*)tree}
      , jobids{std::move(jobids_)}
      , restore_job{restore_job}
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

struct job_started_state {
  JobId_t jobid;
};

struct restore_session_handle {
  std::string error{};
  JobControlRecord* jcr{};
  BareosDb* db{};
  directordaemon::CatalogResource* catalog{};
  directordaemon::JobResource* default_restore_job{};

  struct {
    // TODO: think about how to convert this to JobId_t
    std::vector<int64_t> jobids;
    bool select_parents;
    bool merge_filesets;
  } start;

  const ConfigResourcesContainer& config() const
  {
    return *jcr->dir_impl->job_config_resources_container_.get();
  }

  std::variant<select_start_state,
               build_restore_tree_state,
               select_tree_state,
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

static BareosDb* OpenDb(JobControlRecord* jcr, CatalogResource* catalog)
{
  return DbSqlGetPooledConnection(
      jcr, catalog->db_driver, catalog->db_name, catalog->db_user,
      catalog->db_password.value, catalog->db_address, catalog->db_port,
      catalog->db_socket,
      /* mult_db_conn */ false, catalog->disable_batch_insert,
      catalog->try_reconnect, catalog->exit_on_fatal,
      /* private */ true);
}

directordaemon::JobResource* DefaultRestoreJob(
    const ConfigResourcesContainer& cfg)
{
  auto* job = dynamic_cast<JobResource*>(GetDefaultRes(cfg, R_JOB));

  while (job) {
    if (job->JobType == JT_RESTORE) { return job; }
    job = dynamic_cast<JobResource*>(job->next_);
  }

  return nullptr;
}

restore_session_handle* PluginCreateRestoreSession(const char* catalog_name)
{
  auto* handle = new restore_session_handle{};

  auto* jcr = NewDirectorJcr(DirdFreeJcr);
  if (!jcr) { return nullptr; }
  handle->jcr = jcr;

  handle->catalog = reinterpret_cast<CatalogResource*>(
      GetResWithName(handle->config(), R_CATALOG, catalog_name));
  if (!handle->catalog) {
    handle->error
        = "Cannot begin restore session as no catalog resource exists";
    delete handle;
    return nullptr;
  }

  auto* db = OpenDb(jcr, handle->catalog);
  if (!db) {
    handle->error
        = "Cannot begin restore session as no database connection could be "
          "established";
    delete handle;
    return nullptr;
  }
  handle->db = db;

  handle->default_restore_job = DefaultRestoreJob(handle->config());
  if (!handle->default_restore_job) {
    handle->error = "Cannot start restore session as no restore job exists";
    delete handle;
    return nullptr;
  }

  return handle;
}

const char* PluginErrorString(restore_session_handle* handle)
{
  return handle->error.c_str();
}

bool PluginStartFromJobIds(restore_session_handle* handle,
                           jobid_start_options opts)
{
  auto* state = std::get_if<select_start_state>(&handle->state);
  if (!state) { return false; }

  if (opts.select_parents) {
    handle->error = "Not Implemented Yet";
    return false;
  }

  TreeArgs args;
  args.initial_selection = TreeArgs::selection::None;
  bool first = true;
  std::string jobid_string{};
  for (size_t i = 0; i < opts.count; ++i) {
    auto [_, inserted] = args.jobids.insert(opts.jobids[i]);
    if (inserted) {
      if (!first) {
        jobid_string += ",";
      } else {
        first = false;
      }
      jobid_string += std::to_string(opts.jobids[i]);
    }
  }
  args.estimated_size = 500;  // TODO: fix this

  for (auto id : args.jobids) { handle->start.jobids.push_back(id); }

  auto& build_state = handle->state.emplace<build_restore_tree_state>(
      build_restore_tree_state(static_cast<std::size_t>(args.estimated_size),
                               handle->db, std::move(args)));

  auto ctx = build_state.wait_and_get();
  if (ctx.error) {
    handle->error = "Tree creation error: ERR=" + ctx.error.value();
    handle->start.jobids.clear();
    return false;
  }

  auto* root = ctx.root;
  auto size = ctx.TotalCount;
  handle->state.emplace<select_tree_state>(root, size, std::move(jobid_string),
                                           handle->default_restore_job);

  handle->start.select_parents = opts.select_parents;
  handle->start.merge_filesets = opts.merge_filesets;

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

std::unique_ptr<RestoreBootstrapRecord> MakeBsr(restore_session_handle* handle,
                                                TREE_ROOT* root)
{
  std::unique_ptr bsr = BsrFromTree(root);

  if (!bsr) {
    handle->error = "No files selected to be restored.";
    return nullptr;
  }

  std::optional error
      = AddVolumeInformationToBsr(handle->db, handle->jcr, bsr.get());
  if (error) {
    handle->error = "Could not finalize bsr: ERR=" + error.value();
    return nullptr;
  }

  return bsr;
}

bool PluginCreateRestoreJob(restore_session_handle* handle,
                            job_started_info* info)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  RestoreOptions opts{};

  if (state->restore_client) {
    opts.restore_client = state->restore_client.value();
  }

  if (state->restore_client) {
    opts.restore_client = state->restore_client.value();
  }

  if (state->restore_location) {
    opts.location
        = RestoreOptions::where{std::move(state->restore_location).value()};
  }

  opts.job = state->restore_job;
  opts.replace = state->replace;
  opts.catalog = handle->catalog;

  opts.jobids = state->jobids;

  opts.catalog = handle->catalog;

  if (state->restore_job->Protocol != PT_NATIVE) {
    handle->error = "ndmp support not yet available.";
    return false;
  } else {
    std::unique_ptr bsr = MakeBsr(handle, state->root);
    if (!bsr) { return false; }

    opts.data = RestoreOptions::native_data{
        .bsr = std::move(bsr),
        .bsr_path = std::nullopt,
    };
  }

  auto* jcr = CreateJob(std::move(opts));

  auto jobid = RunJob(jcr);

  if (jobid <= 0) {
    handle->error = "Could not create jcr";
    return false;
  }

  FreeJcr(jcr);

  info->jobid = jobid;

  handle->state = job_started_state{jobid};

  return true;
}

bareos_file_type file_type(unsigned int node_type)
{
  switch (node_type) {
    case TN_ROOT:
    case TN_NEWDIR:
      return BFT_DIR_NOT_BACKED_UP;
    case TN_DIR:
    case TN_DIR_NLS:
      return BFT_DIR;
    case TN_FILE:
      return BFT_FILE;
  }

  ASSERT(!"Unknown node type");
}

file_status FileOfNode(TREE_ROOT* root, TREE_NODE* node)
{
  file_status status = {
      .id = NodeIndex(root, node),
      .name = node->fname ? node->fname : "",
      .type = file_type(static_cast<FILETYPES>(node->type)),
      .marked = node->extract || node->extract_dir,
  };
  return status;
}

bool PluginListFiles(restore_session_handle* handle,
                     size_t* root,
                     file_callback* cb,
                     void* user)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  TREE_NODE* parent = state->current;
  if (root) {
    parent = NodeWithIndex(state->root, *root);
    if (!parent) {
      handle->error = "No node with supplied index";
      return false;
    }
    // should we error when given a file, or just return nothing ?
  }

  TREE_NODE* child = nullptr;

  while ((child = (TREE_NODE*)parent->child.next(child))) {
    file_status status = FileOfNode(state->root, child);
    if (!(*cb)(user, status)) {
      handle->error = "user error";
      return false;
    }
  }

  return true;
}

bool PluginPathSegmentsOf(restore_session_handle* handle,
                          size_t file_id,
                          file_callback* cb,
                          void* user)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  TREE_NODE* node = NodeWithIndex(state->root, file_id);

  if (!node) {
    handle->error = "Could not segmentate: bad file id";
    return false;
  }

  while (node) {
    auto file = FileOfNode(state->root, node);
    if (!cb(user, file)) {
      handle->error = "Fehler";
      return false;
    }
    node = node->parent;
  }

  return true;
}

bool PluginChangeDirectory(restore_session_handle* handle, size_t dir_id)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  TREE_NODE* node = NodeWithIndex(state->root, dir_id);

  if (!node) {
    handle->error = "Could not change dir: bad file id";
    return false;
  }

  if (node->type == TN_FILE) {
    handle->error = "Could not change dir: id does not belong to a directory";
    return false;
  }

  state->current = node;

  return true;
}

bool PluginCurrentDirectory(restore_session_handle* handle, size_t* file_id)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  *file_id = NodeIndex(state->root, state->current);

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

bool PluginPathToFile(restore_session_handle* handle,
                      const char* path,
                      file_status* status)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  auto [dir, file] = split_path_into_dir_file(path);

  if (dir.size() + file.size() == 0) {
    handle->error = "bad path";
    return false;
  }

  TREE_NODE* directory = state->current;

  if (dir.size()) {
    directory = tree_cwd(dir.data(), state->root, state->current);
    if (!directory) {
      handle->error = "bad path: unknown directory";
      return false;
    }
  } else {
    // 'x' could be meant as either "file x" or "directory x", but we always
    // parse it as a file.
    // This means that we should also try to look up the file part if
    // the directory part does not exist.
    // Otherwise we would have to reimplement the behaviour of the special
    // files '.', '..', etc.

    TREE_NODE* tmp = tree_cwd(file.data(), state->root, state->current);
    if (tmp) {
      std::swap(file, dir);
      directory = tmp;
    }
  }

  TREE_NODE* node = nullptr;

  if (file.size()) {
    TREE_NODE* child = nullptr;
    while ((child = static_cast<TREE_NODE*>(directory->child.next(child)))) {
      if (file == child->fname) {
        node = child;
        break;
      }
    }

    if (!node) {
      handle->error = "bad path: unknown file";
      return false;
    }

  } else {
    node = directory;
  }

  *status = FileOfNode(state->root, node);

  return true;
}

bool PluginUpdateRestoreState(restore_session_handle* handle,
                              const restore_options* opts)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  std::optional<JobResource*> restore_job{};
  std::optional<ClientResource*> restore_client{};
  std::optional<replace_option> replace{};
  std::optional<std::string> restore_location{};

  if (opts->restore_job) {
    auto* res = dynamic_cast<JobResource*>(
        GetResWithName(handle->config(), R_JOB, opts->restore_job));
    if (!res) {
      handle->error = std::string{"Job not found: "} + opts->restore_job;
      return false;
    } else {
      restore_job = res;
    }
  }

  if (opts->restore_client) {
    auto* res = dynamic_cast<ClientResource*>(
        GetResWithName(handle->config(), R_CLIENT, opts->restore_client));
    if (!res) {
      handle->error = std::string{"Client not found: "} + opts->restore_client;
      return false;
    } else {
      restore_client = res;
    }
  }

  if (opts->restore_location) { restore_location = opts->restore_location; }

  switch (opts->replace) {
    case REPLACE_FILE_DEFAULT: {
    } break;
    case REPLACE_FILE_ALWAYS: {
      replace = replace_option::Always;
    } break;
    case REPLACE_FILE_IFNEWER: {
      replace = replace_option::IfNewer;
    } break;
    case REPLACE_FILE_IFOLDER: {
      replace = replace_option::IfOlder;
    } break;
    case REPLACE_FILE_NEVER: {
      replace = replace_option::Never;
    } break;
  }

  state->restore_job = restore_job.value_or(handle->default_restore_job);
  state->restore_client = std::move(restore_client);
  state->restore_location = std::move(restore_location);
  state->replace = std::move(replace);

  return true;
}

static uint64_t NumFilesMarked(select_tree_state* st)
{
  // TODO: there has to be a smarter way than this.
  //       the tree should maybe keep track of how many files are marked
  uint64_t files_marked = 0;
  for (auto* node : st->root->alloc) {
    // technically extract/extract_dir already imply in_use, but lets
    // leave it like this for now
    if (node->in_use && (node->extract || node->extract_dir)) {
      files_marked += 1;
    }
  }
  return files_marked;
}

bool PluginCurrentSessionState(restore_session_handle* handle,
                               bareos_session_state* bss)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }

  bss->catalog_name = handle->catalog->resource_name_;
  bss->marked_count = NumFilesMarked(state);
  if (state->restore_client) {
    bss->options.restore_client = state->restore_client.value()->resource_name_;
  }
  bss->options.restore_job = state->restore_job->resource_name_;
  if (state->restore_location) {
    bss->options.restore_location = state->restore_location->c_str();
  }
  if (state->replace) {
    switch (*state->replace) {
      case replace_option::Always: {
        bss->options.replace = REPLACE_FILE_ALWAYS;
      } break;
      case replace_option::IfNewer: {
        bss->options.replace = REPLACE_FILE_IFNEWER;
      } break;
      case replace_option::Never: {
        bss->options.replace = REPLACE_FILE_NEVER;
      } break;
      case replace_option::IfOlder: {
        bss->options.replace = REPLACE_FILE_IFOLDER;
      } break;
    }
  } else {
    bss->options.replace = REPLACE_FILE_DEFAULT;
  }

  bss->start = {
      .count = handle->start.jobids.size(),
      .jobids = handle->start.jobids.data(),
      .select_parents = handle->start.select_parents,
      .merge_filesets = handle->start.merge_filesets,
  };

  return true;
}

bool can_recurse(TREE_NODE* node)
{
  // we can recurse into all non files, as well as into softlinks with children.
  if (node->type != TN_FILE) { return true; }
  if (node->soft_link) { return node->child.size() > 0; }
  return false;
}

void UpdateMarkStatusRecursively(TREE_NODE* node, bool mark)
{
  if (!can_recurse(node)) { return; }


  struct dir_state {
    TREE_NODE* last{nullptr};
    rblist* children{nullptr};

    TREE_NODE* next()
    {
      last = static_cast<TREE_NODE*>(children->next(last));
      return last;
    }

    dir_state() = default;
    dir_state(TREE_NODE* parent) : children{&parent->child} {}
  };

  std::vector<dir_state> stack{node};

  // traverse the file tree in a depth-first way.  This is because there might
  // be a lot of files, but the file tree has comparatively very few levels,
  // which means that dfs should only take very little memory.
  while (!stack.empty()) {
    auto* child = stack.back().next();

    if (child) {
      child->extract = mark;

      if (can_recurse(child)) { stack.emplace_back(child); }
    } else {
      // if there was no child, then we are done. Return for now.
      stack.pop_back();
    }
  }
}

bool PluginMarkUnmark(restore_session_handle* handle,
                      size_t index,
                      bool mark,
                      bool recursive)
{
  auto* state = std::get_if<select_tree_state>(&handle->state);
  if (!state) {
    handle->error = "Wrong state";
    return false;
  }
  TREE_NODE* node = NodeWithIndex(state->root, index);

  if (!node) {
    handle->error = "invalid index";
    return false;
  }


  if (node->extract != mark) { node->extract = mark; }

  if (recursive) { UpdateMarkStatusRecursively(node, mark); }


  return true;
}

namespace config {
bool PluginListClients(config_client_callback* cb, void* user)
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

bool PluginListJobs(config_job_callback* cb, void* user)
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
        .description = job->description_,
    };
    if (!cb(user, &data)) { return false; }
  }
  return true;
}

bool PluginListCatalogs(config_catalog_callback* cb, void* user)
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

bool sendit(void*, const char*, ...) { return true; }

bool PluginClientDefinition(const char* name,
                            config_definition_callback* cb,
                            void* user)
{
  ResLocker _{my_config};

  auto* res = my_config->GetResWithName(R_CLIENT, name);

  if (!res) { return false; }

  OutputFormatter fmt(sendit, nullptr, nullptr, nullptr, API_MODE_JSON);
  OutputFormatterResource fmt_res(&fmt);

  res->PrintConfig(fmt_res, *my_config, true, false);


  auto* json = fmt.JsonResult();

  bool result = true;

  auto* clients = json_object_get(json, "clients");

  if (!clients) { return false; }

  auto* client = json_object_get(clients, name);

  if (!client) { return false; }

  const char* option;
  json_t* value;
  json_object_foreach(client, option, value)
  {
    auto* string = json_string_value(value);

    if (string) {
      if (!cb(user, option, string)) {
        result = false;
        break;
      }
    }
  }

  fmt.FinalizeResult(true);

  return result;
}
bool PluginCatalogDefinition(const char* name,
                             config_definition_callback* cb,
                             void* user)
{
  ResLocker _{my_config};

  auto* res = my_config->GetResWithName(R_CATALOG, name);

  if (!res) { return false; }

  OutputFormatter fmt(sendit, nullptr, nullptr, nullptr, API_MODE_JSON);
  OutputFormatterResource fmt_res(&fmt);

  res->PrintConfig(fmt_res, *my_config, true, false);


  auto* json = fmt.JsonResult();

  bool result = true;

  auto* catalogs = json_object_get(json, "catalogs");

  if (!catalogs) { return false; }

  auto* catalog = json_object_get(catalogs, name);

  if (!catalog) { return false; }

  const char* option;
  json_t* value;
  json_object_foreach(catalog, option, value)
  {
    auto* string = json_string_value(value);

    if (string) {
      if (!cb(user, option, string)) {
        result = false;
        break;
      }
    }
  }

  fmt.FinalizeResult(true);

  return result;
}
bool PluginJobDefinition(const char* name,
                         config_definition_callback* cb,
                         void* user)
{
  ResLocker _{my_config};

  auto* res = my_config->GetResWithName(R_JOB, name);

  if (!res) { return false; }

  OutputFormatter fmt(sendit, nullptr, nullptr, nullptr, API_MODE_JSON);
  OutputFormatterResource fmt_res(&fmt);

  res->PrintConfig(fmt_res, *my_config, true, false);


  auto* json = fmt.JsonResult();

  bool result = true;

  auto* jobs = json_object_get(json, "jobs");

  if (!jobs) { return false; }

  auto* job = json_object_get(jobs, name);

  if (!job) { return false; }

  const char* option;
  json_t* value;
  json_object_foreach(job, option, value)
  {
    auto* string = json_string_value(value);

    if (string) {
      if (!cb(user, option, string)) {
        result = false;
        break;
      }
    }
  }

  fmt.FinalizeResult(true);

  return result;
}
}  // namespace config

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

  void add_field(SQL_FIELD* field, field_flags) override
  {
    fields.emplace_back(strdup(field->name));
  }

  bool handle(SQL_ROW row) override
  {
    return cb(user, fields.size(), fields.data(), row);
  }

  ~sql_result_handler()
  {
    for (auto* field : fields) { free(field); }
  }

  DB_result_callback* cb;
  void* user;

  std::vector<char*> fields;
};

bool ListClients(database_session* sess,
                 DB_result_callback* cb,
                 void* user,
                 const char* outer)
{
  if (!sess) { return false; }

  sess->error.reset();

  sql_result_handler handler(cb, user);

  DbLocker _{sess->ptr};

  auto* inner = sess->ptr->ListClientQuery();

  PoolMem query;
  if (outer) {
    Mmsg(query, outer, inner);
  } else {
    PmStrcpy(query, inner);
  }

  if (!sess->ptr->SqlQuery(query.c_str())) {
    sess->error = sess->ptr->strerror();
    return false;
  }

  if (sess->ptr->ListResult(&handler) < 0) {
    sess->error = sess->ptr->strerror();
    return false;
  }

  return true;
}

bool ListJobs(database_session* sess,
              DB_result_callback* cb,
              void* user,
              const char* outer)
{
  if (!sess) { return false; }

  sess->error.reset();

  sql_result_handler handler(cb, user);

  DbLocker _{sess->ptr};

  auto* inner = sess->ptr->ListJobsQuery();

  PoolMem query;
  if (outer) {
    Mmsg(query, outer, inner);
  } else {
    PmStrcpy(query, inner);
  }

  if (!sess->ptr->SqlQuery(query.c_str())) {
    sess->error = sess->ptr->strerror();
    return false;
  }

  if (sess->ptr->ListResult(&handler) < 0) {
    sess->error = sess->ptr->strerror();
    return false;
  }

  return true;
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
            .path_segments_of = &PluginPathSegmentsOf,
            .create_restore_session = &PluginCreateRestoreSession,
            .start_from_jobids = &PluginStartFromJobIds,
            .finish_restore_session = &PluginFinishRestoreSession,
            .create_restore_job = &PluginCreateRestoreJob,
            .path_to_file = &PluginPathToFile,

            .update_restore_state = &PluginUpdateRestoreState,
            .current_session_state = &PluginCurrentSessionState,
        };
        memcpy(buffer, &cap, sizeof(cap));
        return true;
      }
    } break;
    case CAP_Config: {
      if (check_buffer<config_capability>(bufsize, buffer)) {
        config_capability cap = {
            .list_clients = &config::PluginListClients,
            .list_jobs = &config::PluginListJobs,
            .list_catalogs = &config::PluginListCatalogs,
            .client_definition = &config::PluginClientDefinition,
            .job_definition = &config::PluginJobDefinition,
            .catalog_definition = &config::PluginCatalogDefinition,
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

  if (!directory) {
    Dmsg0(100, "No plugin directory set. Skipping.\n");
    return false;
  }

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
