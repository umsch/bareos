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

#include "grpc.h"

using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

template <typename Callback>
bool sql_callback_helper(size_t num_fields,
                         const char* const fields[],
                         const char* const rows[],
                         void* user)
{
  auto* cb = reinterpret_cast<Callback*>(user);
  return (*cb)(num_fields, fields, rows);
}

class ClientImpl : public Client::Service {
 public:
  ClientImpl(client_capability rc) : cap{rc} {}

 private:
  Status List(ServerContext*,
              const ListClientsRequest*,
              ServerWriter<SqlResponse>* writer) override
  {
    auto cb = [writer](size_t num_fields, const char* const fields[],
                       const char* const rows[]) -> bool {
      SqlResponse response;
      for (size_t i = 0; i < num_fields; ++i) {
        auto [_, inserted]
            = response.mutable_row()->insert({fields[i], rows[i]});
        if (!inserted) { return false; }
      }
      writer->Write(response);
      return true;
    };
    if (!cap.list_clients(sql_callback_helper<decltype(cb)>, &cb)) {
      return Status(grpc::StatusCode::UNKNOWN, "Internal error");
    }

    return grpc::Status::OK;
  }

  Status Info(ServerContext*,
              const ClientInfoRequest* request,
              SqlResponse* response) override
  {
    bool found = false;
    auto cb = [&found, response](size_t num_fields, const char* const fields[],
                                 const char* const rows[]) -> bool {
      if (found) { return false; }
      for (size_t i = 0; i < num_fields; ++i) {
        auto [_, inserted]
            = response->mutable_row()->insert({fields[i], rows[i]});
        if (!inserted) { return false; }
      }
      found = true;
      return true;
    };
    if (!cap.client_info(request->clientname().c_str(),
                         sql_callback_helper<decltype(cb)>, &cb)) {
      return Status(grpc::StatusCode::UNKNOWN, "Internal error");
    }
    if (!found) {
      return Status(grpc::StatusCode::UNKNOWN, "No client with that name");
    }
    return Status::OK;
  }

  client_capability cap;
};

std::unique_ptr<Client::Service> MakeClientService(client_capability cap)
{
  return std::make_unique<ClientImpl>(cap);
}
