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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_CONFIG_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

typedef bool(config_name_callback)(void* user, const char* name);

typedef bool(ConfigListClients_t)(config_name_callback* cb, void* user);
typedef bool(ConfigListJobs_t)(config_name_callback* cb, void* user);
typedef bool(ConfigListCatalogs_t)(config_name_callback* cb, void* user);

struct config_capability {
  ConfigListClients_t* list_clients;
  ConfigListJobs_t* list_jobs;
  ConfigListCatalogs_t* list_catalogs;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_CONFIG_H_
