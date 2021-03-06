/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2019-2019 Bareos GmbH & Co. KG

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

#include "gtest/gtest.h"
#include "include/bareos.h"
#include "lib/parse_conf.h"
#include "dird/dird_globals.h"
#include "dird/dird_conf.h"

namespace directordaemon {

TEST(ConfigParser_Dir, bareos_configparser_tests)
{
  InitMsg(NULL, NULL); /* initialize message handler */

  std::string path_to_config_file = std::string(
      PROJECT_SOURCE_DIR "/src/tests/configs/bareos-configparser-tests");
  my_config = InitDirConfig(path_to_config_file.c_str(), M_ERROR_TERM);
  my_config->ParseConfig();

  my_config->DumpResources(PrintMessage, NULL);

  delete my_config;

  TermMsg();         /* Terminate message handler */
  CloseMemoryPool(); /* release free memory in pool */
}

TEST(ConfigParser_Dir, runscript_test)
{
  InitMsg(NULL, NULL); /* initialize message handler */

  std::string path_to_config_file = std::string(
      PROJECT_SOURCE_DIR "/src/tests/configs/runscript-tests/bareos-dir.conf");
  my_config = InitDirConfig(path_to_config_file.c_str(), M_ERROR_TERM);
  my_config->ParseConfig();

  my_config->DumpResources(PrintMessage, NULL);

  delete my_config;

  TermMsg();         /* Terminate message handler */
  CloseMemoryPool(); /* release free memory in pool */
}

}  // namespace directordaemon
