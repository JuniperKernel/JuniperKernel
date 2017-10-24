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

  zmq::pollitem_t items[n + 1 /* one more for the inproc signaller*/];
  zmq::socket_t* signaller = subscribe_to(context, INPROC_SIG);
  items[0] = {*signaller, 0, ZMQ_POLLIN, 0 };
  for(int i=1; i<=n; i++)
    items[i] = {*sockets[i-1], 0, ZMQ_POLLIN, 0};

  bool dead=false;
  while( !dead ) {
    zmq::poll(items, n+1, -1);
    std::function<bool()>* hp = handlers;
    if( items[0].revents & ZMQ_POLLIN ) {  // got a kill signal
      dead=true;
      break;
    }
    try {
    for(int i=1; i<=n; i++)
      if( items[i].revents & ZMQ_POLLIN )
        if( (dead=!hp[i-1]()) )
          break;
    } catch(const std::exception& x) {
      std::cout << "Encountered C++ exception: " <<  x.what() << std::endl;
      std::cout << "Polling continues..." << std::endl;
    }
  }

  assert(dead);
  
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
