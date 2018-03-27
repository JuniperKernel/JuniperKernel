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
#ifndef juniper_juniper_juniper_H
#define juniper_juniper_juniper_H

#define LINGER 1000 // number of millis to linger for
#define INPROC_SIG "inproc://controller"  // the death signaller from xeus

#ifdef ERROR
#undef ERROR
#endif

#include <Rcpp.h>
#include <iostream>
#include <atomic>
#include <string>
#include <thread>
#include <xeus/xguid.hpp>
#include <xeus/xinterpreter.hpp>
#include <juniper/utils.h>
#include <juniper/sockets.h>
#include <RInside.h>

using xeus::xinterpreter;
using xeus::xjson;
using xeus::xjson_node;
using xeus::xhistory_arguments;

class JadesInterpreter: public xinterpreter {
  public:
    JadesInterpreter(RInside* rin):
      _rin(rin),
      _ctx(new zmq::context_t(1)),
      _connected(0) {
        _sout_sock = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
        _serr_sock = listen_on(*_ctx, "tcp://*:*", zmq::socket_type::stream);
        _sout_port = read_port(_sout_sock);
        _serr_port = read_port(_serr_sock);
        _sout_thread = stream_thread(_sout_sock, true, _connected, _ctx);
        _serr_thread = stream_thread(_serr_sock, false, _connected,_ctx);
      }

    ~JadesInterpreter() {
      if( _ctx ) delete _ctx;
      _sout_thread.join();
      _serr_thread.join();
    }

    void configure_impl(){ register_interpreter(this); }

    xjson R_perform(const std::string& request_type, xjson& req) {
      req["stream_out_port"] = _sout_port;
      req["stream_err_port"] = _serr_port;
      SEXP ans;
      Rcpp::List req_msg = from_json_r(req);
      (*_rin)["__jk_req_msg__"] = req_msg;
      (*_rin)["__jk_req_type__"] = request_type;
      _rin->parseEval("JuniperKernel::doRequest()", ans);
      std::string res = Rcpp::as<std::string>(ans);
      xjson p = xjson::parse(res);

      int ntries=0;
      int breakcnt=0;
      int MAXBREAKS=5;
      while( _connected.load() ) {
        if( ntries++ % 10000000 == 0 ) breakcnt++;
        if( breakcnt > MAXBREAKS ) {
          Rcpp::Rcout << "WARNING: R socketConnection disconnects not detected." << std::endl;
          _connected = 0;
        }
      }
      return p;
    }

    xjson execute_request_impl(
      int execution_counter,
      const std::string& code,
      bool silent,
      bool store_history,
      const xjson_node* /* user_expressions */,
      bool allow_stdin) {
        xjson req;
        req["code"] = code;
        req["execution_counter"] = execution_counter;
        return R_perform("execute_request", req);
    }

    xjson complete_request_impl(
      const std::string& code,
      int cursor_pos) {
        xjson req;
        req["code"] = code;
        req["cursor_pos"] = cursor_pos;
        return R_perform("complete_request", req);
    }

    xjson inspect_request_impl(
      const std::string& code,
      int cursor_pos,
      int detail_level) {
        xjson req;
        req["code"] = code;
        req["cursor_pos"] = cursor_pos;
        req["detail_level"] = detail_level;
        return R_perform("inspect_request", req);
    }

    xjson history_request_impl(const xhistory_arguments& args) {
      xjson req;
      return R_perform("history_request", req);
    }

    xjson is_complete_request_impl(const std::string& code) {
      xjson req;
      req["code"] = code;
      return R_perform("is_complete_request", req);
    }

    xjson kernel_info_request_impl() {
      xjson req;
      return R_perform("kernel_info_request", req);
    }

    void input_reply_impl(const std::string& value) {
    }

  private:
    RInside* const _rin;
    zmq::context_t* const _ctx;

    zmq::socket_t* _sout_sock;
    zmq::socket_t* _serr_sock;
    int _sout_port;
    int _serr_port;
    std::thread _sout_thread;
    std::thread _serr_thread;
    mutable std::atomic<int> _connected;


    std::thread stream_thread(
      zmq::socket_t* sock,
      const bool out,
      std::atomic<int>& connected,
      zmq::context_t* ctx
    ) const {
      std::thread out_thread([sock, out, &connected, ctx]() {  // full fence membar
        short conn=false;
        std::function<bool()> handlers[] = {
          [sock, out, &connected, &conn, ctx]() {
            zmq::multipart_t msg;
            msg.recv(*sock);

            // connect/disconnect logic
            // in either case it's a two-frame multipart message:
            //   frame 1: identity
            //   frame 2: empty
            // the first time getting these two frames means we're
            // getting connected to by a socket having identity found in
            // frame1. the second time means the socket having that
            // identity disconnected. Our expectation is that we shall
            // only ever have a single connection; and when it dies, we also
            // die (AKA signal death to the poller by returning false).
            if( msg[1].size()==0 ) {
              if( !conn++ )
                connected.fetch_add(1);
                else {
                  conn=false;
                  connected.fetch_sub(1);
                }
              return true;
            }
            // read the message and publish to stdout
            xeus::get_interpreter().publish_stream(out?"stdout":"stderr", msg_t_to_string(msg[1]));
            return true;
          }
        };
        zmq::socket_t* socket[1] = {sock};
        poll(*ctx, socket, handlers, 1);
      });
      return out_thread;
    }
};

#endif // ifndef juniper_juniper_juniper_H
