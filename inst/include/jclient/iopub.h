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


class IOPub {
  public:
    long long *_msg_ctr;  // count messages received
    std::string _port;
    std::thread _io_t;

    const IOPub& start_iopub(zmq::context_t* ctx) {
      zmq::socket_t* sock = subscribe_to(*ctx, "tcp://*:*");
      _port = std::move(read_port(sock));
      long long ctr = *_msg_ctr;
      std::thread io_thread([sock, ctx, &ctr]() {
        std::function<bool()> handlers[] = {
          [sock, &ctr]() {
            zmq::multipart_t msg;
            msg.recv(*sock);
            if( msg[1].size()==0 )
              return true;  // (dis)connects are ignored

            ctr++;  // bump the message count
            std::cout << "IOPUB: " << msg_t_to_string(msg[1]) << std::endl;
            return true;
          }
        };
        zmq::socket_t* sockets[1] = {sock};
        poll(*ctx, sockets, handlers, 1);
      });
      _io_t = std::move(io_thread);
      return *this;
    }

    const long long message_count() const { return *_msg_ctr; }
};