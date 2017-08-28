#ifndef juniper_juniper_background_H
#define juniper_juniper_background_H

#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "sockets.h"


void start_hb_thread(zmq::context_t& ctx, const std::string& endpoint) {
  std::thread hb([&ctx, endpoint]() {
    Rcpp::Rcout << "hb_endpoint: " << endpoint << std::endl;

    zmq::socket_t* hbSock = listen_on(ctx, endpoint, zmq::socket_type::rep);  // bind to the heartbeat endpoint
    zmq::socket_t* sockets[] = { hbSock };
    std::function<bool()> handlers[] = {
      // ping-pong the message on heartbeat
      [&hbSock]() {
        zmq::multipart_t msg;
        msg.recv(*hbSock);
        msg.send(*hbSock);
        return true;
      }
    };
    poll(ctx, sockets, handlers, 1);
  });
  hb.detach();
}

void start_io_thread(zmq::context_t& ctx, const std::string& endpoint) {
  std::thread io([&ctx, endpoint]() {
      Rcpp::Rcout << "IO_endpoint: " << endpoint << std::endl;

    zmq::socket_t* io = listen_on(ctx, endpoint, zmq::socket_type::pub);  // bind to the iopub endpoint
    zmq::socket_t* pubsub = subscribe_to(ctx, INPROC_PUB); // subscription to internal publisher
    zmq::socket_t* sockets[] = { pubsub };

    std::function<bool()> handlers[] = {
      // msg forwarding
      [&pubsub, &io]() {
        // we've got some messages to send from the
        // execution engine back to the client. Messages
        // from the executor are published to the inproc_pub
        // topic, and we forward them to the client here.
        zmq::multipart_t msg;
        msg.recv(*pubsub);
        msg.send(*io);
        return true;
      }
    };
    poll(ctx, sockets, handlers, 1);
    io->setsockopt(ZMQ_LINGER, 0);
    delete io;
  });
  io.detach();
}

#endif // #ifndef juniper_juniper_background_H
