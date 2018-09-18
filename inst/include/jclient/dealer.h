// Copyright (C) 2017-2018  Spencer Aiello
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
#ifndef juniper_jclient_dealer_H
#define juniper_jclient_dealer_H
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <zmq/zmq.h>
#include <zmq/zmq.hpp>
#include <juniper/sockets.h>
#include <juniper/utils.h>
#include <juniper/jmessage.h>

class DealerSocket {
  public:
    zmq::socket_t* _sock=NULL;
    int _port;

    void init_socket(zmq::context_t* ctx, int port) {
      _port=port;
      _sock = new zmq::socket_t(*ctx, zmq::socket_type::dealer);
      _sock->connect("tcp://127.0.0.1:" + std::to_string(port));
    }

    void close() {
      if( _sock!=nullptr ) {
        _sock->setsockopt(ZMQ_LINGER, 0);
        delete _sock;
      }
    }
};
#endif // #ifndef juniper_jclient_dealer_H
