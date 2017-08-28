#ifndef juniper_juniper_sockets_H
#define juniper_juniper_sockets_H

#include <cassert>
#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "juniper.h"

zmq::socket_t* init_socket(zmq::socket_t* socket, const std::string& endpoint) {

  Rcpp::Rcout << " init socket endpoint: " << endpoint << std::endl;

  socket->setsockopt(ZMQ_LINGER, LINGER);
  socket->bind(endpoint);
  return socket;
}

zmq::socket_t* subscribe_to(zmq::context_t& context, const std::string& topic) {

  Rcpp::Rcout << "sub to topic: " << topic << std::endl;

  // connect to the topic signaller
  zmq::socket_t* sub = new zmq::socket_t(context, zmq::socket_type::sub);
  // for option ZMQ_SUBSCRIBE, no need to set before connect (but we do anyways)
  // see: http://api.zeromq.org/4-1:zmq-setsockopt
  sub->setsockopt(ZMQ_SUBSCRIBE, "" /*no filter*/, 0);
  sub->connect(topic);
  return sub;
}

zmq::socket_t* listen_on(zmq::context_t& context, const std::string& endpoint, zmq::socket_type type) {
  zmq::socket_t* sock = new zmq::socket_t(context, type);
  sock->setsockopt(ZMQ_LINGER, LINGER);
  return init_socket(sock, endpoint);
}

// Functional style polling with custom message handling
// if a handler returns false, it signals to stop polling
// also handles all thread-local socket teardown logic
void poll(zmq::context_t& context, zmq::socket_t* sockets[], std::function<bool()> handlers[], int n) {
  zmq::socket_t* sp = *sockets;

  zmq::pollitem_t items[n + 1 /* one more for the inproc signaller*/];
  zmq::socket_t* signaller = subscribe_to(context, INPROC_SIG);
  items[0] = {*signaller, 0, ZMQ_POLLIN, 0 };
  for(int i=1; i<=n; i++)
    items[i] = {*sp++, 0, ZMQ_POLLIN, 0};
  
  bool dead=false;
  while( !dead ) {
    zmq::poll(items, n+1, -1);

    std::function<bool()>* hp = handlers;

    // check if we got a kill signal
    if( items[0].revents & ZMQ_POLLIN ) {
      Rcpp::Rcout << "suicide" << std::endl;
      dead=true; // our loop invariant
      break; // breaks the while
    }

    for(int i=1; i<=n; i++)
      if( items[i].revents & ZMQ_POLLIN )
        if( !(*hp++)() ) {
          dead=true;
          break;  // breaks the for
        }
  }

  assert(dead);
  
  // cleanup
  // loop over sockets, set their lingers to 0, and delete
  signaller->setsockopt(ZMQ_LINGER, 0);
  delete signaller;

  for(int i=0; i<n; i++) {
    sockets[i]->setsockopt(ZMQ_LINGER, 0);
    delete sockets[i];
  }
}

#endif // #ifndef juniper_juniper_sockets_H
