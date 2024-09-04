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
#include <stdint.h>

enum bareos_job_type : uint32_t
{
  BJT_BACKUP = 'B',       /**< Backup Job */
  BJT_MIGRATED_JOB = 'M', /**< A previous backup job that was migrated */
  BJT_VERIFY = 'V',       /**< Verify Job */
  BJT_RESTORE = 'R',      /**< Restore Job */
  BJT_CONSOLE = 'U',      /**< console program */
  BJT_SYSTEM = 'I',       /**< internal system "job" */
  BJT_ADMIN = 'D',        /**< admin job */
  BJT_ARCHIVE = 'A',      /**< Archive Job */
  BJT_JOB_COPY = 'C',     /**< Copy of a Job */
  BJT_COPY = 'c',         /**< Copy Job */
  BJT_MIGRATE = 'g',      /**< Migration Job */
  BJT_SCAN = 'S',         /**< Scan Job */
  BJT_CONSOLIDATE = 'O'   /**< Always Incremental Consolidate Job */
};


enum bareos_job_level : uint32_t
{
  BJL_NONE = 0,
  BJL_FULL = 'F',         /* Full backup */
  BJL_INCREMENTAL = 'I',  /* since last backup */
  BJL_DIFFERENTIAL = 'D', /* since last full backup */
};

struct bareos_config_catalog {
  const char* name;
  const char* db_name;
};

struct bareos_config_option {
  const char* name;
  const char* value;
};

struct bareos_config_job {
  const char* name;
  enum bareos_job_type type;

  const char* description;  // may be null!
};

struct bareos_config_client {
  const char* name;
  const char* address;
};

typedef bool(config_client_callback)(void* user,
                                     const bareos_config_client* data);
typedef bool(config_catalog_callback)(void* user,
                                      const bareos_config_catalog* data);
typedef bool(config_job_callback)(void* user, const bareos_config_job* data);
typedef bool(config_definition_callback)(void* user,
                                         const char* option,
                                         const char* value);

typedef bool(ConfigListClients_t)(config_client_callback* cb, void* user);
typedef bool(ConfigListJobs_t)(config_job_callback* cb, void* user);
typedef bool(ConfigListCatalogs_t)(config_catalog_callback* cb, void* user);

typedef bool(ConfigClientDefinition_t)(const char* client_name,
                                       config_definition_callback* cb,
                                       void* user);
typedef bool(ConfigJobDefinition_t)(const char* job_name,
                                    config_definition_callback* cb,
                                    void* user);
typedef bool(ConfigCatalogDefinition_t)(const char* catalog_name,
                                        config_definition_callback* cb,
                                        void* user);

struct config_capability {
  ConfigListClients_t* list_clients;
  ConfigListJobs_t* list_jobs;
  ConfigListCatalogs_t* list_catalogs;

  ConfigClientDefinition_t* client_definition;
  ConfigJobDefinition_t* job_definition;
  ConfigCatalogDefinition_t* catalog_definition;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_CONFIG_H_
