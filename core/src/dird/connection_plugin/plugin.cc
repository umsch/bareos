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
#include "dir.h"
#include "plugin.h"
#include "dird/dird_conf.h"
#include "dird/dird_globals.h"
#include "dird/job.h"
#include "dird/jcr_util.h"
#include "include/baconfig.h"

#include <memory>
#include <optional>
#include <dlfcn.h>

#include "lib/parse_conf.h"
#include "list_clients.h"

namespace directordaemon {
void Log(log_severity severity, const char* str)
{
  Dmsg2(500, "%d: %s\n", severity, str);
}

bool PluginListClients(list_client_callback* cb, void* user)
{
  auto* jcr = NewDirectorJcr(DirdFreeJcr);

  // ua->db = DbSqlGetPooledConnection(
  //     ua->jcr, ua->catalog->db_driver, ua->catalog->db_name,
  //     ua->catalog->db_user, ua->catalog->db_password.value,
  //     ua->catalog->db_address, ua->catalog->db_port, ua->catalog->db_socket,
  //     mult_db_conn, ua->catalog->disable_batch_insert,
  //     ua->catalog->try_reconnect, ua->catalog->exit_on_fatal, use_private);
  // DbSqlGetPooledConnection();

  auto* db = [jcr]() -> BareosDb* {
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
  }();

  if (!db) { return false; }

  // db->ListClientRecords(jcr, NULL, formatter, RAW_LIST);

  //   ;

  DbSqlClosePooledConnection(jcr, db);

  (void)jcr;
  (void)cb;
  (void)user;
  return false;
}

bool PluginListClient(const char* name, list_client_callback* cb, void* user)
{
  (void)name;
  (void)cb;
  (void)user;
  return false;
}

bool QueryCabability(bareos_capability Cap, size_t bufsize, void* buffer)
{
  switch (Cap) {
    case CAP_ListClients: {
      if (bufsize == sizeof(struct list_client_capability)
          && std::align(alignof(struct list_client_capability), bufsize, buffer,
                        bufsize)) {
        list_client_capability cap = {
            .list_clients = &PluginListClients,
            .list_client = &PluginListClient,
        };
        memcpy(buffer, &cap, sizeof(cap));
        return true;
      }
    } break;
    case CAP_Restore:
      return false;
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


  int port = 13343;  // todo: get from config
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
