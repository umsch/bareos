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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_LIST_CLIENTS_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_LIST_CLIENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef bool(sql_callback)(size_t num_fields,
                           char const* const fields[],
                           char const* const row[],
                           void* user);

typedef bool(ListClients_t)(sql_callback* cb, void* user);
typedef bool(ListClient_t)(const char* client_name,
                           sql_callback* cb,
                           void* user);

struct list_client_capability {
  ListClients_t* list_clients;
  ListClient_t* list_client;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_LIST_CLIENTS_H_
