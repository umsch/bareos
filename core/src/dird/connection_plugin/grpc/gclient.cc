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

#include <grpcpp/grpcpp.h>
#include <iostream>

#include "restore.grpc.pb.h"
#include "config.grpc.pb.h"
#include "database.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class Client {
 public:
  Client(std::shared_ptr<Channel> ch)
      : db{bareos::database::Database::NewStub(ch)}
  {
  }

  using db_stub = bareos::database::Database::Stub;
  std::unique_ptr<db_stub> db;

  void test()
  {
    using namespace bareos::database;

    bareos::config::CatalogId cid;
    cid.set_name("MyCatalog");

    ListJobsRequest req;
    *req.mutable_catalog() = cid;

    ClientContext ctx;

    ListJobsResponse resp;

    auto status = db->ListJobs(&ctx, req, &resp);

    if (!status.ok()) {
      std::cout << "Error/" << status.error_code() << ": "
                << status.error_message() << "\n  " << status.error_details()
                << std::endl;
      return;
    }

    for (auto& job : resp.jobs()) {
      std::cout << "Job: " << job.id().id() << std::endl;
    }
    std::cout << "OK" << std::endl;
  }
};

int main()
{
  Client client(
      grpc::CreateChannel("0.0.0.0:13343", grpc::InsecureChannelCredentials()));

  client.test();

  return 1;
}
