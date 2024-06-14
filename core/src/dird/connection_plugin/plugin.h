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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_PLUGIN_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_PLUGIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>   // for size_t
#include <stdbool.h>  // for bool

enum log_severity
{
  LOG_DEBUG = 0,
  LOG_INFO = 1,
  LOG_WARNING = 2,
  LOG_ERROR = 3,
  LOG_FATAL = 4,
};

typedef void (*Log_t)(enum log_severity, const char*);

enum bareos_capability
{
  CAP_Client = 0,
  CAP_Restore = 1,
  CAP_Config = 2,
};

typedef bool (*QueryCapability_t)(enum bareos_capability,
                                  size_t bufsize,
                                  void* buffer);

struct bareos_api {
  size_t size;
  Log_t log;
  QueryCapability_t query;
};

typedef bool (*loadPlugin_t)(const bareos_api*);
typedef bool (*unloadPlugin_t)();
typedef bool (*Start_t)(int port);

struct plugin_api {
  size_t size;  // = sizeof(plugin_api)
  const char* license;

  loadPlugin_t load;
  unloadPlugin_t unload;
  Start_t start;
};

#define CONN_PLUGIN_API_SYMBOL_NAME plugin_api_for_bareos

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_PLUGIN_H_
