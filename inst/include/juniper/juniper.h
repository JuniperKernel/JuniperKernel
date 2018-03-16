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

#ifdef ERROR
#undef ERROR
#endif
#include <Rcpp.h>
#include <zmq_addon.hpp>

#define LINGER 1000 // number of millis to linger for
#define INPROC_SIG "inproc://controller"  // the death signaller from xeus

#include <zmq.hpp>
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
    JadesInterpreter():
      _ctx(new zmq::context_t(1)),
      _connected(0),
      _jk("package:JadesKernel")
        {}

    ~JadesInterpreter() {
      if( _ctx )
        delete _ctx;
    }

    void configure_impl(){}

    xjson R_perform(const std::string& request_type, xjson& req) {
      Rcpp::Function handler = _jk[request_type];
      Rcpp::Function do_request = _jk["doRequest"];
      Rcpp::List res = do_request(Rcpp::wrap(handler), from_json_r(req));
      return from_list_r(res);
    }

    xjson execute_request_impl(
      int execution_counter,
      const std::string& code,
      bool silent,
      bool store_history,
      const xjson_node* /* user_expressions */,
      bool allow_stdin) {
        Rcpp::Rcout << "EXECUTING CODE: " << code << std::endl;
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
      Rcpp::Rcout << "history request is unimpl." << std::endl;
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
      Rcpp::Rcout << "Received input_reply" << std::endl;
      Rcpp::Rcout << "value: " << value << std::endl;
    }

  private:
    zmq::context_t* const _ctx;
//    RInside* const _rin;
    std::atomic<int> _connected;
    const Rcpp::Environment _jk;
};

#endif // ifndef juniper_juniper_juniper_H
