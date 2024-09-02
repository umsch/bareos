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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_DATABASE_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_DATABASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

struct database_session;

typedef bool(DB_result_callback)(void* user,
                                 size_t field_count,
                                 const char* const* fields,
                                 const char* const* rows);

typedef struct database_session*(DB_OpenDatabase_t)(const char* catalog_name);
typedef void(DB_CloseDatabase_t)(struct database_session*);
typedef const char*(DB_ErrorString_t)(struct database_session*);
typedef bool(DB_ListClients_t)(struct database_session*,
                               DB_result_callback* cb,
                               void* user,
                               /* outer query should contain one %s.
                                * It is formatted with the query to select all
                                * clients as single argument.*/
                               const char* outer);
typedef bool(DB_ListJobs_t)(struct database_session*,
                            DB_result_callback* cb,
                            void* user);
struct database_capability {
  DB_OpenDatabase_t* open_database;
  DB_CloseDatabase_t* close_database;
  DB_ListClients_t* list_clients;
  DB_ListJobs_t* list_jobs;
  DB_ErrorString_t* error_string;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_DATABASE_H_
