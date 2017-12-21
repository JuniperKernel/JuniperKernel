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
#ifndef juniper_jclient_iopub_H
#define juniper_jclient_iopub_H
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <zmq.hpp>
#include <juniper/sockets.h>
#include <juniper/utils.h>
#include <Rcpp.h>


class IOPub {
  public:
    int _port;
    std::thread _io_t;

    IOPub& start_iopub(zmq::context_t* ctx, int port) {
      _port=port;
      zmq::socket_t* sock = new zmq::socket_t(*ctx, zmq::socket_type::sub);
      sock->setsockopt(ZMQ_SUBSCRIBE, "" /*no filter*/, 0);
      sock->connect("tcp://127.0.0.1:" + std::to_string(port));
      std::thread io_thread([sock, ctx]() mutable {
        zmq::socket_t* iopub = listen_on(*ctx, "inproc://iopub", zmq::socket_type::pub);
        std::function<bool()> handlers[] = {
          [sock, iopub]() mutable {
          std::string _key = "cc496d37-59a9-4c61-8900-d826985f564d";
            zmq::multipart_t msg;
            msg.recv(*sock);
            if( msg[1].size()==0 ) {
              Rcpp::Rcout << "IOPub connect/disconnect" << std::endl;
              return true;  // (dis)connects are ignored
            }
            std::string msg_str = JMessage::read(msg, _key).get().dump();
            zmq::multipart_t pubmsg;
            pubmsg.add(zmq::message_t(msg_str.begin(), msg_str.end()));
            pubmsg.send(*iopub);
            return true;
          }
        };
        zmq::socket_t* sockets[1] = {sock};
        poll(*ctx, sockets, handlers, 1);
        iopub->setsockopt(ZMQ_LINGER,0);
        delete iopub;
      });
      _io_t = std::move(io_thread);
      return *this;
    }
};
#endif // #ifndef juniper_jclient_iopub_H
