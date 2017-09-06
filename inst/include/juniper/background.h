#ifndef juniper_juniper_background_H
#define juniper_juniper_background_H

#include <Rcpp.h>
#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <juniper/sockets.h>
#include <juniper/juniper.h>


// extern uintptr_t R_CStackLimit;

std::thread start_hb_thread(zmq::context_t& ctx, const std::string& endpoint) {
  std::thread hbthread([&ctx, endpoint]() {
    zmq::socket_t* hbSock = listen_on(ctx, endpoint, zmq::socket_type::rep);  // bind to the heartbeat endpoint
    std::function<bool()> handlers[] = {
      // ping-pong the message on heartbeat
      [&hbSock]() {
        std::cout << "heartbeat received" << std::endl;
        zmq::multipart_t msg;
        msg.recv(*hbSock);
        msg.send(*hbSock);
        return true;
      }
    };
    poll(ctx, (zmq::socket_t*[]){hbSock}, handlers, 1);
  });
  return hbthread;
}

std::thread start_io_thread(zmq::context_t& ctx, const std::string& endpoint) {
  std::thread iothread([&ctx, endpoint]() {
    zmq::socket_t* io = listen_on(ctx, endpoint, zmq::socket_type::pub);  // bind to the iopub endpoint
    zmq::socket_t* pubsub = subscribe_to(ctx, INPROC_PUB); // subscription to internal publisher
    std::function<bool()> handlers[] = {
      // msg forwarding
      [&pubsub, &io]() {
        // we've got some messages to send from the
        // execution engine back to the client. Messages
        // from the executor are published to the inproc_pub
        // topic, and we forward them to the client here.
        zmq::multipart_t msg;
        msg.recv(*pubsub);
        std::cout << "iopub msg: " << msg.str() << std::endl;
        msg.send(*io);
        return true;
      },
      [] {
        assert(false);  // we should never be run...
        return true;
      }
    };
    poll(ctx, (zmq::socket_t*[]){pubsub, io}, handlers, 2);
  });
  return iothread;
}

std::thread start_T_thread(zmq::context_t& ctx) {
  // R_CStackLimit = (uintptr_t)-1;
  std::thread Tthread([&ctx]() {
    zmq::socket_t* sock = new zmq::socket_t(ctx, zmq::socket_type::stream);
    sock->connect("tcp://localhost:6011");
    std::function<bool()> handlers[] = {
      [&sock]() {
        zmq::multipart_t msg;
        msg.recv(*sock);
        std::cout << "T thread msg: " << msg.str() << std::endl;
        return true;
      }
    };
    poll(ctx, (zmq::socket_t* []){sock}, handlers, 1);
  });
  return Tthread;
}

#endif // #ifndef juniper_juniper_background_H
