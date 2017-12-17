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
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <zmq.hpp>
#include <juniper/sockets.h>
#include <juniper/utils.h>
#include <juniper/jmessage.h>

class Shell {
  public:
    const std::string _key;
    zmq::socket_t* _sock;

    int _port;
    Shell(){}
    Shell(const std::string& key, zmq::context_t* ctx): _key(key), _sock(listen_on(*ctx, "tcp://*:*", zmq::socket_type::dealer)) {
      _port = read_port(_sock);
    }
    ~Shell() {
      _sock->setsockopt(ZMQ_LINGER,0);
      delete _sock;
      Rcpp::Rcout << "Shell dead" << std::endl;
    }

    std::string execute_request(std::string req) {
      zmq::multipart_t msg;
      msg.add(zmq::message_t(req.begin(), req.end()));
      msg.send(*_sock);
      zmq::multipart_t res;
      res.recv(*_sock);
      return JMessage::read(res, _key).get().dump();
    }
};