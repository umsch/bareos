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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_RESTORE_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_RESTORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

struct restore_session_handle;

struct job_started_info {
  int64_t jobid;
};

struct file_status {
  const char* name;
  bool marked;
};

typedef struct restore_session_handle*(CreateRestoreSession_t)(void);
typedef bool(StartFromJobIds_t)(struct restore_session_handle*,
                                size_t count,
                                const int64_t jobids[],
                                bool select_parents);
typedef bool(file_callback)(void* user, struct file_status status);
typedef bool(ListFiles_t)(struct restore_session_handle*,
                          file_callback* cb,
                          void* user);
typedef bool(ChangeDirectory_t)(struct restore_session_handle*,
                                const char* dir);
typedef bool(MarkUnmark_t)(struct restore_session_handle*,
                           const char* regex,
                           bool mark,
                           file_callback* cb,
                           void* user);
typedef const char*(ErrorString_t)(struct restore_session_handle*);
typedef bool(SetRestoreClient_t)(struct restore_session_handle*,
                                 const char* clientname);


typedef void(AbortRestoreSession_t)(struct restore_session_handle*);
typedef bool(CommitRestoreSession_t)(struct restore_session_handle*,
                                     struct job_started_info*);

struct restore_capability {
  CreateRestoreSession_t* create_restore_session;
  ListFiles_t* list_files;
  ChangeDirectory_t* change_directory;
  MarkUnmark_t* mark_unmark;
  ErrorString_t* error_string;
  StartFromJobIds_t* start_from_jobids;
  SetRestoreClient_t* set_restore_client;
  AbortRestoreSession_t* abort_restore_session;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_RESTORE_H_
