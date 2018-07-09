/*
   BAREOS® - Backup Archiving REcovery Open Sourced

   Copyright (C) 2018-2018 Bareos GmbH & Co. KG

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
#include <iostream>
#include <fstream>
#include "include/bareos.h"

extern std::string d_msg_srcfile_filter_expression; /* regex to filter the debug output */

std::string myname = "d_msg_unittest";
std::string working_dir = "/tmp";
std::string tracefilename = working_dir + "/" + myname + ".trace";

static void init_for_test() {
  std::remove(tracefilename.c_str());
  debug_level = 3;
  working_directory = working_dir.c_str();
  MyNameIs(0, NULL, myname.c_str());
  InitMsg(NULL, NULL);

  SetTrace(0); // sync file
  SetTrace(1);
}

std::string read_one_line() {
  std::ifstream file(tracefilename);
  std::string logline;
  getline(file, logline);
  file.close();
  return logline;
}

TEST(d_msg, output_below_debug_level) {
  init_for_test();
  d_msg("dir/file", 12345, 1, "This is a log message\n");
  ASSERT_EQ(read_one_line(), "d_msg_unittest (1): dir/file:12345-0 This is a log message");
}

TEST(d_msg, output_equal_to_debug_level) {
  init_for_test();
  d_msg("dir/file", 12345, 3, "This is a log message\n");
  ASSERT_EQ(read_one_line(), "d_msg_unittest (3): dir/file:12345-0 This is a log message");
}

TEST(d_msg, noout_higher_than_debug_level) {
  init_for_test();
  d_msg("dir/file", 12345, 5, "This is a log message\n");
  ASSERT_TRUE(read_one_line().empty());
}

// tests for regex filtering
TEST(d_msg, noout_regex_doesnt_match) {
  init_for_test();
  d_msg_srcfile_filter_expression = "dir/blub";
  d_msg("dir/file", 12345, 1, "This is a log message\n");

  ASSERT_TRUE(read_one_line().empty());
  d_msg_srcfile_filter_expression = "";
}

TEST(d_msg, output_regex_match) {
  init_for_test();
  d_msg_srcfile_filter_expression = "dir/.*";
  d_msg("filed/file", 12345, 1, "This is a log message\n");
  d_msg("stored/file", 12345, 1, "This is a log message\n");
  d_msg("dir/file", 12345, 1, "This is a log message\n");
  d_msg("console/file", 12345, 1, "This is a log message\n");

  ASSERT_EQ(read_one_line(), "d_msg_unittest (1): dir/file:12345-0 This is a log message");
  d_msg_srcfile_filter_expression = "";
}
