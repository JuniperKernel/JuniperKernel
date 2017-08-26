#ifndef juniper_juniper_background_H
#define juniper_juniper_background_H

#include "sockets.h"
#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

void start_hb_thread(zmq::context_t& ctx, const std::string endpoint, const std::string inproc_sig) {
  std::thread hb([&ctx, &endpoint, &inproc_sig]() {
    // bind to the heartbeat endpoint
    zmq::socket_t hb(ctx, zmq::socket_type::rep);
    init_socket(&hb, endpoint);

    zmq::socket_t sigsub = subscribe_to(ctx, inproc_sig);

    // setup pollitem_t instances and poll
    zmq::pollitem_t items[] = { 
      {sigsub, 0, ZMQ_POLLIN, 0}, 
      {hb,     0, ZMQ_POLLIN, 0}
    };  

    std::function<bool()> handlers[] = { 
      // define what happens when we get a poison pill
      [&sigsub, &hb]() {
        return false;
      },  
      // ping-pong the message on heartbeat
      [&hb]() {
        zmq::multipart_t msg;
        msg.recv(hb);
        msg.send(hb);
        return true;
      }   
    };  
    poller(items, handlers, 2); 
  }); 
}

void start_io_thread(zmq::context_t& ctx, const std::string endpoint, const std::string inproc_sig, const std::string inproc_pub) {
  std::thread io([&ctx, &endpoint, &inproc_sig, &inproc_pub]() {
    // bind to the iopub endpoint
    zmq::socket_t io(ctx, zmq::socket_type::pub);
    init_socket(&io, endpoint);

    zmq::socket_t sigsub = subscribe_to(ctx, inproc_sig);
    zmq::socket_t pubsub = subscribe_to(ctx, inproc_pub);

    // setup pollitem_t instances and poll
    zmq::pollitem_t items[] = { 
      {sigsub, 0, ZMQ_POLLIN, 0}, 
      {pubsub, 0, ZMQ_POLLIN, 0}
    };  

    std::function<bool()> handlers[] = { 
      // got a poison pill; cleanup sockets
      [&sigsub, &pubsub, &io]() {
        return false;
      },  
      // msg forwarding
      [&pubsub, &io]() {
        // we've got some messages to send from the
        // execution engine back to the client. Messages
        // from the executor are published to the inproc_pub
        // topic, and we forward them to the client here.
        zmq::multipart_t msg;
        msg.recv(pubsub);
        msg.send(io);
        return true;
      },  
    };  
    poller(items, handlers, 2); 
  }); 
}

#endif // #ifndef juniper_juniper_background_H
