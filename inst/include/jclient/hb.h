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
#ifndef juniper_jclient_hb_H
#define juniper_jclient_hb_H
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
#include <Rcpp.h>

const std::string PING = "ping";
const int HEARTBEAT_INTERVAL_SECS = 60;

void beat(zmq::socket_t* sock) {
  zmq::multipart_t msg;
  msg.add(zmq::message_t(PING.begin(), PING.end()));
  msg.send(*sock);
}

class HB {
  public:
    int _port;
    std::thread _hb_t;
    volatile bool _beating;

    const HB& start_hb(zmq::context_t* ctx, int port) {
      _port=port;
      zmq::socket_t* sock = new zmq::socket_t(*ctx, zmq::socket_type::req);
      sock->connect("tcp://127.0.0.1:" + std::to_string(port));
      volatile bool* is_beating = &_beating;
      std::thread hb_thread([sock, ctx, is_beating]() {
        bool dead=false;
        zmq::socket_t* signaller = subscribe_to(*ctx, INPROC_SIG);
        try {
          zmq::multipart_t conn;
          beat(sock);
          conn.recv(*sock); // wait for a connection, then enter the loop
          *is_beating=true;
          zmq::pollitem_t items[] = {
            {*signaller, 0, ZMQ_POLLIN, 0},
            {*sock, 0, ZMQ_POLLIN, 0}
          };
          int no_hb=0;
          while( !dead ) {
            zmq::poll(&items[0], 1, HEARTBEAT_INTERVAL_SECS*1000);  // heartbeat
            if( (dead=(items[0].revents & ZMQ_POLLIN)) )
              break;
            beat(sock);
            zmq::poll(items, 2, 1000/*millis*/);
            if( (dead=(items[0].revents & ZMQ_POLLIN)) )
              break;

            if( items[1].revents & ZMQ_POLLIN ) {
              zmq::multipart_t rec;
              rec.recv(*sock);
              Rcpp::Rcout << "HB: " << read_str(rec.pop()) << std::endl;
              continue;
            }

            if( no_hb > 2 )
              break;
          }
        } catch( const std::exception& x ) {
          Rcpp::Rcout << "exception in HB thread: " << x.what() << std::endl;
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
#endif // #ifndef juniper_jclient_hn_H