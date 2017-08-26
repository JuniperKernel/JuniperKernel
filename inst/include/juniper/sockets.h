#ifndef juniper_juniper_sockets_H
#define juniper_juniper_sockets_H

#include <string>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "juniper.h"

zmq::socket_t subscribe_to(zmq::context_t& context, const std::string& inproc_topic) {
  // connect to the inproc signaller
  zmq::socket_t sub(context, zmq::socket_type::sub);
  // for option ZMQ_SUBSCRIBE, no need to set before connect (but we do anyways)
  // see: http://api.zeromq.org/4-1:zmq-setsockopt
  sub.setsockopt(ZMQ_SUBSCRIBE, "" /*no filter*/, 0); 
  sub.connect(inproc_topic);
  return sub;
}

void init_socket(zmq::socket_t* socket, std::string endpoint) {
  socket->setsockopt(ZMQ_LINGER, LINGER);
  socket->bind(endpoint);
}

// Functional style polling with custom message handling
// if a handler returns false, it signals to stop polling
// and this will cleanup and delete all of the sockets
void poller(zmq::pollitem_t items[], std::function<bool()> handlers[], int n) {
  while( 1 ) { 
    zmq::poll(&items[0], n, -1);
    for(int i=0;i<n;++i)
      if( items[i].revents & ZMQ_POLLIN)
        if( !handlers[i]() )
          break; // got a poison pill
  }

  // cleanup
  // loop over sockets, set their lingers to 0, and delete
  for(int i=0;i<n;i++) {
    zmq::socket_t* sock = (zmq::socket_t*)items[i].socket;
    sock->setsockopt(ZMQ_LINGER, 0); 
    delete sock;
  }
}

#endif // #ifndef juniper_juniper_sockets_H
