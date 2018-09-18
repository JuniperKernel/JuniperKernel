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
#ifndef juniper_juniper_requests_H
#define juniper_juniper_requests_H

#include <atomic>
#include <string>
#include <thread>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
#include <Rcpp.h>
#include <juniper/sockets.h>
#include <juniper/conf.h>
#include <juniper/jmessage.h>
#include <juniper/utils.h>

// Every incoming message has a type, which tells the server which handler
// the message should be passed to for further execution.
using buffer_sequence = std::vector<zmq::message_t>;
class RequestServer {
  public:
    mutable JMessage _cur_msg;  // messages are handled 1 at a time; keep a ref
    RequestServer(zmq::context_t& ctx, const std::string& key):
      _ctx(&ctx),
      _key(key)
        {
          // these are internal routing sockets that push messages (e.g.
          // poison pills, results, etc.) to the heartbeat thread and
          // iopub thread.
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
          _stream_out = listen_on(ctx, "tcp://*:*", zmq::socket_type::stream);
          _stream_err = listen_on(ctx, "tcp://*:*", zmq::socket_type::stream);
          _stream_out_port = read_port(_stream_out);
          _stream_err_port = read_port(_stream_err);
          _sout = stream_thread(&ctx, _stream_out, true, this);
          _serr = stream_thread(&ctx, _stream_err, false, this);
        }

    ~RequestServer() {
      _serr.join();
      _sout.join();
      _inproc_sig->setsockopt(ZMQ_LINGER, 0); delete _inproc_sig;
      _inproc_pub->setsockopt(ZMQ_LINGER, 0); delete _inproc_pub;
    }

    // handle all client requests here. Every request gets the same
    // treatment:
    //  1. deserialize the incoming message and validate
    //  2. 'busy' is published over iopub
    //  3. the message is passed to a handler
    //  4. a zmq::multipart_t is returned containing the 'content' and 'msg_type' fields
    //  5. send the multipart_t over the socket
    //  6. 'idle' is published over iopub
    SEXP serve(zmq::multipart_t& request, zmq::socket_t& sock) const {
      _cur_msg = std::move(JMessage::read(request, _key));  // read and validate
      return busy().handle();
    }

    void post_handle(Rcpp::List res, zmq::socket_t& sock) const {
      json req = _cur_msg.get();
      std::string msg_type = req["header"]["msg_type"];

      // comms don't reply (except for comm_info_request)
      if( msg_type=="comm_open" || msg_type=="comm_close" || msg_type=="comm_msg" ) {
        idle();
        return;
      }

      json jres = from_list_r(res);

      JMessage::reply(_cur_msg, jres["msg_type"], jres["content"]).send(sock);
      if( msg_type.compare("shutdown_request")==0 ) {
        idle(); // publish idle before triggering socket deaths
        shutdown();
      }
      idle();
    }

    zmq::multipart_t stream_outerr(const std::string& o, const std::string& name) const {
      return JMessage::reply(_cur_msg, "stream", {{"name", name}, {"text", o}});
    }
    void rebroadcast_input(const std::string& input, const int count) const {
       iopub("execute_input", {{"code", input}, {"execution_count", count}});
    }
    void execute_result(const json& data) const { iopub("execute_result", data); }
    void display_data(const json& data) const   { iopub("display_data"  , data); }
    void shutdown() const { zmq::message_t m(0); _inproc_sig->send(m); }
  	void iopub(const std::string& msg_type, const json& content,
							 const json& metadata, buffer_sequence buffers) const {
	  	JMessage::reply(_cur_msg, msg_type, content, metadata, std::move(buffers)).send(*_inproc_pub);
    }
  	void iopub(const std::string& msg_type, const json& content, const json& metadata=json({})) const {
			iopub(msg_type, content, metadata, std::vector<zmq::message_t>());
    }
    const RequestServer& busy() const { iopub("status", {{"execution_state", "busy"}}); return *this; }
    const RequestServer& idle() const { iopub("status", {{"execution_state", "idle"}}); return *this; }

  private:
    zmq::context_t* const _ctx;
    const std::string _key;   // hmac key
    zmq::socket_t* _inproc_pub;
    zmq::socket_t* _inproc_sig;
    zmq::socket_t* _stream_out;
    zmq::socket_t* _stream_err;
    int _stream_out_port;
    int _stream_err_port;

    std::thread _sout;
    std::thread _serr;

    const SEXP handle() const {
      json req = _cur_msg.get();
      req["stream_out_port"] = _stream_out_port;  // stitch the stdout port into the client request
      req["stream_err_port"] = _stream_err_port;  // stitch the stderr into the client request
      req["message_type"] = req["header"]["msg_type"];
      return from_json_r(req);
    }

    std::thread stream_thread(zmq::context_t* ctx, zmq::socket_t* sock, const bool out, RequestServer* rs) const {
      std::thread out_thread([rs, ctx, sock, out]() {  // full fence membar
        zmq::socket_t* pubsock = listen_on(*ctx, out?INPROC_OUT_PUB:INPROC_ERR_PUB, zmq::socket_type::pub);
        std::function<bool()> handlers[] = {
          [rs, sock, out, pubsock]() {
            zmq::multipart_t msg;
            msg.recv(*sock);

            if( msg[1].size()==0 )  // empty message usually denotes (dis)connections
              return true;

            std::string name = out ? "stdout":"stderr";
            std::string msg0 = msg_t_to_string(msg[1]);

            // read the message and publish to the internal out/err publishers
            zmq::multipart_t outerr_msg = rs->stream_outerr(msg0, name);
            outerr_msg.send(*pubsock);
            return true;
          }
        };
        zmq::socket_t* socket[1] = {sock};
        poll(*(rs->_ctx), socket, handlers, 1);
        // cleanup out/err internal pubs
        pubsock->setsockopt(ZMQ_LINGER, 0);
        delete pubsock;
      });
      return out_thread;
    }
};

#endif // ifndef juniper_juniper_requests_H
