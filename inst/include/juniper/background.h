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
#ifndef juniper_juniper_background_H
#define juniper_juniper_background_H

#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <juniper/sockets.h>
#include <juniper/conf.h>


std::thread start_hb_thread(zmq::context_t& ctx, const std::string& endpoint) {
  std::thread hbthread([&ctx, endpoint]() {
    zmq::socket_t* hbSock = listen_on(ctx, endpoint, zmq::socket_type::rep);  // bind to the heartbeat endpoint
    std::function<bool()> handlers[] = {
      // ping-pong the message on heartbeat
      [&hbSock]() {
        zmq::multipart_t msg;
        msg.recv(*hbSock);
        msg.send(*hbSock);
        return true;
      }
    };
    zmq::socket_t* sock[1] = {hbSock};
    poll(ctx, sock, handlers, 1);
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
        // std::cout << "iopub msg: " << msg.str() << std::endl;
        msg.send(*io);
        return true;
      },
      [] { assert(false); return false; /* only here to keep handler same shape as sockets*/ }
    };
    zmq::socket_t* sock[2] = {pubsub, io};
    poll(ctx, sock, handlers, 2);
  });
  return iothread;
}

#endif // #ifndef juniper_juniper_background_H
