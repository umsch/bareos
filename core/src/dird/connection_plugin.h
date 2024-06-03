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

#ifndef BAREOS_DIRD_CONNECTION_PLUGIN_H_
#define BAREOS_DIRD_CONNECTION_PLUGIN_H_

#include <vector>
#include <string>

namespace directordaemon {

bool LoadConnectionPlugins(const char* directory,
                           const std::vector<std::string>& names);
void UnloadConnectionPlugins();

}  // namespace directordaemon
#endif  // BAREOS_DIRD_CONNECTION_PLUGIN_H_
