// Copyright (C) 2017  Spencer Aiello
//
// This file is part of JuniperKernel.
//
// JuniperKernel is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// JuniperKernel is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with JuniperKernel.  If not, see <http://www.gnu.org/licenses/>.
#ifndef juniper_jclient_shell_H
#define juniper_jclient_shell_H
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <zmq.hpp>
#include <json.hpp>
#include <juniper/sockets.h>
#include <juniper/utils.h>
#include <juniper/jmessage.h>
#include <jclient/dealer.h>

class Shell: public DealerSocket {
  public:
    const std::string _key = "cc496d37-59a9-4c61-8900-d826985f564d";
    const std::string _id = "8487E89F214545E19FFADD3C8D56E4AC";
    const std::string _del= "<IDS|MSG>";

    void execute_request(std::string req) {
      json request = json::parse(req);
      zmq::multipart_t msg;
      msg.add(zmq::message_t(_id.begin(), _id.end()));
      msg.add(zmq::message_t(_del.begin(), _del.end()));
      std::string hmac = compute_hmac(request);
      msg.add(zmq::message_t(hmac.begin(), hmac.end()));
      std::string header = request["header"].dump();
      std::string parent_header = request["parent_header"].dump();
      std::string metadata = request["metadata"].dump();
      std::string content = request["content"].dump();
      msg.add(zmq::message_t(header.begin(), header.end()));
      msg.add(zmq::message_t(parent_header.begin(), parent_header.end()));
      msg.add(zmq::message_t(metadata.begin(), metadata.end()));
      msg.add(zmq::message_t(content.begin(), content.end()));
      msg.send(*_sock);
    }

    std::string execute_reply() {
      zmq::multipart_t res;
      res.recv(*_sock);
      return JMessage::read(res, _key).get().dump();
    }

    std::string compute_hmac(const json& req) {
      std::stringstream data;
      data << req["header"].dump();
      data << req["parent_header"].dump();
      data << req["metadata"].dump();
      data << req["content"].dump();
      return hmac<SHA256>(data.str(), _key);
    }
};
#endif // #ifndef juniper_jclient_shell_H