// Copyright (C) 2017  Spencer Aiello
//
// This file is part of JadesKernel.
//
// JadesKernel is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// JadesKernel is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with JadesKernel.  If not, see <http://www.gnu.org/licenses/>.
#ifndef jades_jades_jades_H
#define jades_jades_jades_H

#include <xeus/xinterpreter.hpp>
#include <Rcpp.h>

using xeus::xinterpreter;
using xeus::xjson;
using xeus::xjson_node;
using xeus::xhistory_arguments;

class JadesInterpreter: public xinterpreter {
  public:
    JadesInterpreter() = default;
    virtual ~JadesInterpreter() = default;

  private:
    zmq::context_t* const _ctx;
    zmq::socket_t* _inproc_pub;
    zmq::socket_t* _inproc_sig;
    zmq::socket_t* _stream_out;
    zmq::socket_t* _stream_err;
    int _stream_out_port;
    int _stream_err_port;
    const Rcpp::Environment _jk;
    std::atomic<int> _connected;

    void configure_impl() override;

    xjson execute_request_impl(
      int execution_counter,
      const std::string& code,
      bool silent,
      bool store_history,
      const xjson_node* user_expressions,
      bool allow_stdin) override;

    xjson complete_request_impl(
      const std::string& code,
      int cursor_pos) override;

    xjson inspect_request_impl(
      const std::string& code,
      int cursor_pos,
      int detail_level) override;

    xjson history_request_impl(const xhistory_arguments& args) override;
    xjson is_complete_request_impl(const std::string& code) override;
    xjson kernel_info_request_impl() override;
    void input_reply_impl(const std::string& value) override;
  };
}

#endif // ifndef jades_jades_jades_H
