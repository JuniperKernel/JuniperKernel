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
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <json.hpp>
#include <juniper/sockets.h>
#include <zmq.h>
#include <zmq.hpp>

class JupyterTestClient {
  public:
    zmq::context_t* const _ctx;
    zmq::socket_t* _inproc_sig;
    std::thread _hb_t;
    std::thread _shell_t;
    std::thread _iopub_t;
    std::thread _stdin_t;
    std::thread _ctrl_t;

    JupyterTestClient(): _ctx(new zmq::context_t(1)) {
      _inproc_sig = listen_on(*_ctx, INPROC_SIG, zmq::socket_type::pub);
    }
    void start_client() {
      // 'hb' : zmq.REQ,
      // 'shell' : zmq.DEALER,
      // 'iopub' : zmq.SUB,
      // 'stdin' : zmq.DEALER,
      // 'control': zmq.DEALER

      zmq::socket_t
      _iopub_t = IOPub().start_iopub(_ctx, )
    }



};


class IOPub {
  public:
    long long _msg_ctr;  // count messages received
    IOPub() { _msg_ctr(0); }
    std::thread start_iopub(zmq::context_t* ctx, zmq::socket_t* sock) const {
      std::thread io_thread([sock, ctx, &_msg_ctr)]() {
        std::function<bool()> handlers[] = {
          short conn=false;
          [sock, &_msg_ctr, &conn]() {
            zmq::multipart_t msg;
            msg.recv(*sock);
            if( msg[1].size()==0 )
              return true;  // (dis)connects are ignored

            _msg_ctr++;  // bump the message count
            std::cout << "IOPUB: " << msg_t_to_string(msg[1]) << std::endl;
            return true;
          }
        };
        zmq::socket_t* sockets[1] = {sock};
        poll(*ctx, sockets, handlers, 1);
      });
      return io_thread;
    }
};


          _inproc_pub = listen_on(ctx, INPROC_PUB, zmq::socket_type::pub);
          _inproc_sig = listen_on(ctx, INPROC_SIG, zmq::socket_type::pub);

          // R sinks stdout/stderr to socketConnection, which is being listened
          // to on a ZMQ_STREAM socket. Polling of this connection happens in a
          // separate thread so that stdout/stderr can be streamed to IOPub
          // asynchronously. This socket is created in the main thread and
          // migrated to a polling thread. Why? R needs to know the port--which
          // is chosen by the OS--to form the socketConnection: because R is
          // executing in the main thread there is the option to block the main
          // thread until it handshakes with the spawned thread via inproc
          // transports; or migrate a socket to a new thread. Option 2 is
          // simpler and totally legal according to the ZMQ docs since the socket
          // creation and port scraping happens before the polling thread is spawned.
          _stream_out = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
          _stream_err = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
          _stream_out_port = read_port(_stream_out);
          _stream_err_port = read_port(_stream_err);
          std::thread sout = stream_thread(_stream_out, true, _connected );
          std::thread serr = stream_thread(_stream_err, false, _connected);
          sout.detach();
          serr.detach();