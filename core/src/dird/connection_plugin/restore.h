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

enum bareos_file_type
{
  BFT_FILE = 0,
  BFT_DIR = 1,
  BFT_DIR_NOT_BACKED_UP = 2,
};

struct file_status {
  size_t id;
  const char* name;
  bareos_file_type type;
  bool marked;
};

enum file_replace_option
{
  REPLACE_FILE_DEFAULT = 0,
  REPLACE_FILE_ALWAYS,
  REPLACE_FILE_IFNEWER,
  REPLACE_FILE_IFOLDER,
  REPLACE_FILE_NEVER,
};

struct restore_options {
  enum file_replace_option replace;
  const char* restore_job;
  const char* restore_location;
  const char* restore_client;
};

struct bareos_session_state {
  const char* catalog_name;
  int64_t backup_id;
  bool can_restore;
  restore_options options;
};

typedef const char*(ErrorString_t)(struct restore_session_handle*);
typedef void(FinishRestoreSession_t)(struct restore_session_handle*);
typedef struct restore_session_handle*(CreateRestoreSession_t)(void);
typedef bool(StartFromJobIds_t)(struct restore_session_handle* handle,
                                size_t count,
                                const int64_t jobids[],
                                bool select_parents);
typedef bool(file_callback)(void* user, struct file_status status);
typedef bool(ListFiles_t)(struct restore_session_handle*,
                          size_t* root_id,
                          file_callback* cb,
                          void* user);
typedef bool(ChangeDirectory_t)(struct restore_session_handle*,
                                size_t directory_id);
typedef bool(MarkUnmark_t)(struct restore_session_handle*,
                           size_t index,
                           bool mark,
                           bool recursive);

typedef bool(PathSegmentsOf_t)(struct restore_session_handle*,
                               size_t file_id,
                               file_callback* cb,
                               void* user);
typedef bool(CurrentDirectory_t)(struct restore_session_handle*,
                                 size_t* file_id);

typedef bool(key_value_handler)(void* user, const char* key, const char* value);

typedef bool(CreateRestoreJob_t)(struct restore_session_handle*,
                                 job_started_info* info);

typedef bool(PathToFile_t)(struct restore_session_handle*,
                           const char* path,
                           struct file_status* status);

typedef bool(UpdateRestoreState_t)(struct restore_session_handle*,
                                   const struct restore_options* opts);

typedef bool(CurrentSessionState_t)(struct restore_session_handle*,
                                    struct bareos_session_state* bss);

struct restore_capability {
  ListFiles_t* list_files;
  ChangeDirectory_t* change_directory;
  MarkUnmark_t* mark_unmark;
  ErrorString_t* error_string;
  CurrentDirectory_t* current_directory;
  PathSegmentsOf_t* path_segments_of;

  CreateRestoreSession_t* create_restore_session;
  StartFromJobIds_t* start_from_jobids;
  FinishRestoreSession_t* finish_restore_session;
  CreateRestoreJob_t* create_restore_job;

  PathToFile_t* path_to_file;

  UpdateRestoreState_t* update_restore_state;
  CurrentSessionState_t* current_session_state;
};

#ifdef __cplusplus
}
#endif

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_RESTORE_H_
