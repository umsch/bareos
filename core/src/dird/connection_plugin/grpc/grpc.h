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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_GRPC_GRPC_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_GRPC_GRPC_H_

#include <memory>
#include "dird/connection_plugin/restore.h"
#include "dird/connection_plugin/database.h"
#include "dird/connection_plugin/config.h"
#include "restore.grpc.pb.h"
#include "config.grpc.pb.h"
#include "database.grpc.pb.h"

std::unique_ptr<bareos::restore::Restore::Service> MakeRestoreService(
    restore_capability cap);
std::unique_ptr<bareos::config::Config::Service> MakeConfigService(
    config_capability cap);
std::unique_ptr<bareos::database::Database::Service> MakeDatabaseService(
    database_capability cap);

template <typename... Ts> void ignore(Ts&&...) {}

template <typename Callback, typename... Args>
auto c_callback(void* user, Args... args)
{
  auto* cb = reinterpret_cast<Callback*>(user);
  return (*cb)(args...);
}


struct source_location {
  const char* file;
  const char* function;
  int line;

  constexpr source_location(const char* f = __builtin_FILE(),
                            const char* fun = __builtin_FUNCTION(),
                            int l = __builtin_LINE()) noexcept
      : file{f}, function{fun}, line{l}
  {
  }
};

struct grpc_error : public std::exception {
  grpc::Status status;

  grpc_error(grpc::StatusCode code,
             const std::string& message,
             const std::string& desc)
      : status(code, message, desc)
  {
  }
  grpc_error(grpc::StatusCode code,
             const std::string& message,
             source_location loc = source_location{})
      : grpc_error(code,
                   message,
                   "at " + std::string(loc.file) + ":"
                       + std::to_string(loc.line) + "(" + loc.function + ")")
  {
  }
};

#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_GRPC_GRPC_H_
