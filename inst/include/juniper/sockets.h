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
#ifndef juniper_juniper_sockets_H
#define juniper_juniper_sockets_H

#include <cassert>
#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <juniper/conf.h>

zmq::socket_t* init_socket(zmq::socket_t* socket, const std::string& endpoint) {
  socket->setsockopt(ZMQ_LINGER, LINGER);
  socket->bind(endpoint);
  return socket;
}

zmq::socket_t* subscribe_to(zmq::context_t& context, const std::string& topic) {
  zmq::socket_t* sub = new zmq::socket_t(context, zmq::socket_type::sub);
  sub->setsockopt(ZMQ_SUBSCRIBE, "" /*no filter*/, 0);
  sub->connect(topic);
  return sub;
}

zmq::socket_t* listen_on(zmq::context_t& context, const std::string& endpoint, zmq::socket_type type) {
  zmq::socket_t* sock = new zmq::socket_t(context, type);
  return init_socket(sock, endpoint);
}

// Polling logic + teardown on signal in a single place. Functional
// style polling with custom message handling if a handler returns
// false, stop polling and teardown.
// All callers get an additional signal listener.
void poll(zmq::context_t& context, zmq::socket_t* sockets[], std::function<bool()> handlers[], int n) {
  zmq::pollitem_t* items = (zmq::pollitem_t*)malloc((n+1/* one more for the inproc signaller*/)*sizeof(zmq::pollitem_t));
  zmq::socket_t* signaller = subscribe_to(context, INPROC_SIG);
  items[0] = {*signaller, 0, ZMQ_POLLIN, 0};
  for( int i=1; i<=n; i++ )
    items[i] = {*sockets[i-1], 0, ZMQ_POLLIN, 0};

  bool dead=false;
  while( !dead ) {
    try {
      zmq::poll(items, n+1, -1);
      std::function<bool()>* hp = handlers;
      if( (dead=(items[0].revents & ZMQ_POLLIN)) )  // got a kill signal
        break;

      try {
        for( int i=1; i<=n; i++ )
          if( items[i].revents & ZMQ_POLLIN )
            if( (dead=!hp[i-1]()) )
              break;
      } catch(const std::exception& x) {
        Rcpp::Rcout << "Encountered C++ exception: " <<  x.what() << std::endl;
        Rcpp::Rcout << "Polling continues..." << std::endl;
      }
    } catch(zmq::error_t& e) { /*ignored*/ }
  }

  assert(dead);
  free(items);

  // cleanup
  // loop over sockets, set their lingers to 0, and delete
  signaller->setsockopt(ZMQ_LINGER, 0);
  delete signaller;

  for(int i=0; i<n; i++) {
    zmq::socket_t* sock = sockets[i];
    sock->setsockopt(ZMQ_LINGER, 0);
    delete sock;
  }
}

#endif // #ifndef juniper_juniper_sockets_H
