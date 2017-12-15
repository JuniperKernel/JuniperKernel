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
#include <chrono>
#include <zmq.h>
#include <zmq.hpp>
#include <juniper/sockets.h>
#include <juniper/utils.h>
#include <juniper/conf.h>


class HB {
  public:
    std::string _port;
    std::thread _hb_t;

    const HB& start_hb(zmq::context_t* ctx) {
      zmq::socket_t* sock = listen_on(*ctx, "tcp://*:*", zmq::socket_type::req);
      _port = std::move(read_port(sock));

      std::thread hb_thread([sock, ctx]() {
        bool dead=false;
        zmq::socket_t* signaller = subscribe_to(*ctx, INPROC_SIG);
        zmq::multipart_t conn;
        conn.recv(*sock); // wait for a connection, then enter the loop
        std::cout << "connected; heart beat started" << std::endl;
        int no_hb=0;
        std::string ping="ping";
        while( !dead ) {
          std::this_thread::sleep_for(std::chrono::seconds(30));
          zmq::multipart_t msg;
          msg.add(zmq::message_t(ping.begin(), ping.end()));
          msg.send(*sock);

          zmq::pollitem_t items[] = {
            {*signaller, 0, ZMQ_POLLIN, 0},
            {*sock, 0, ZMQ_POLLIN, 0}
          };
          zmq::poll(items, 2, 1000/*seconds*/);

          if( (dead=(items[0].revents & ZMQ_POLLIN)) )
            break;

          if( items[1].revents & ZMQ_POLLIN ) {
            zmq::multipart_t rec;
            std::cout << "HB: " << rec.recv(*sock) << std::endl;
            continue;
          }

          std::cout << "No heart beat..." << no_hb << std::endl;
          if( no_hb > 2 )
            break;
        }

        signaller->setsockopt(ZMQ_LINGER,0);
        delete signaller;
        sock->setsockopt(ZMQ_LINGER,0);
        delete sock;
      });
      _hb_t = std::move(hb_thread);
      return *this;
    }
};