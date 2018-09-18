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
#ifndef juniper_juniper_juniper_H
#define juniper_juniper_juniper_H

#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
#include <nlohmann/json.hpp>
#include <juniper/conf.h>
#include <juniper/sockets.h>
#include <juniper/background.h>
#include <juniper/requests.h>
#include <juniper/external.h>

class JuniperKernel {
  public:
    const RequestServer* _request_server;
    JuniperKernel(const config& conf):
      _ctx(new zmq::context_t(1)),

      // these are the 3 incoming Jupyter channels
      _stdin(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
      _hbport(conf.hb_port),
      _ioport(conf.iopub_port),
      _shellport(conf.shell_port),
      _cntrlport(conf.control_port),
      _key(conf.key),
      _sig_scheme(conf.signature_scheme) {
        _request_server = new RequestServer(*_ctx, _key);
        char sep = (conf.transport=="tcp") ? ':' : '-';
        _endpoint = conf.transport + "://" + conf.ip + sep;

        // socket setup
        init_socket(_stdin, _endpoint + conf.stdin_port);
    }

    static JuniperKernel* make(const std::string& connection_file) {
      config conf = config::read_connection_file(connection_file);
      JuniperKernel* jk = new JuniperKernel(conf);
      return jk;
    }

    // start the background threads
    // called as part of the kernel boot sequence
    SEXP start_bg_threads(int interrupt_event) {
      start_intr_thread(interrupt_event);
      _hbthread = start_hb_thread(*_ctx, _endpoint + _hbport);
      _iothread = start_io_thread(*_ctx, _endpoint + _ioport);
      Rcpp::List cfg(2);
      Rcpp::StringVector names(2);
      _cntrl = listen_on(*_ctx, _endpoint + _cntrlport, zmq::socket_type::router, true);
      _shell = listen_on(*_ctx, _endpoint + _shellport, zmq::socket_type::router, true);
      names(0) = "ctl";
      names(1) = "shl";
      cfg[0] = makeExternalSocketPtr(_cntrl);
      cfg[1] = makeExternalSocketPtr(_shell);
      cfg.attr("names") = names;
      return Rcpp::wrap(cfg);
    }

    SEXP makeExternalSocketPtr(zmq::socket_t* sock) {
      SEXP R_socket = R_NilValue;
      PROTECT(R_socket = R_MakeExternalPtr(*sock, R_NilValue, R_NilValue));
      UNPROTECT(1);
      return R_socket;
    }

    void serve(zmq::multipart_t& msg, zmq::socket_t& sock) {
      _request_server->serve(msg, sock);
    }

    SEXP recv(const std::string& sockName) {
      zmq::socket_t* sock = sockName.compare("control")==0 ? _cntrl: _shell;
      zmq::multipart_t msg;
      msg.recv(*sock);
      return _request_server->serve(msg, *sock);
    }

    void post_handle(Rcpp::List res, const std::string& sockName) {
      zmq::socket_t* sock = sockName.compare("control")==0 ? _cntrl: _shell;
      _request_server->post_handle(res, *sock);
    }

    ~JuniperKernel() {
      // set linger to 0 on all sockets; destroy sockets; finally destoy ctx
      _request_server->shutdown(); // try to send a signal out again
      _hbthread.join();
      _iothread.join();
      delete _request_server;
      _stdin->setsockopt(ZMQ_LINGER, 0); delete _stdin;
      if( _shell ) {
        _shell->setsockopt(ZMQ_LINGER, 0);
        delete _shell;
      }
      if( _cntrl ) {
        _cntrl->setsockopt(ZMQ_LINGER, 0);
        delete _cntrl;
      }
      delete _ctx;
    }

  private:
    // context is shared by all threads, cause there
    // ain't no GIL to stop us now! ...we can build this thing together!
    zmq::context_t* const _ctx;

    // jupyter stdin
    zmq::socket_t*  const _stdin;

    zmq::socket_t* _shell;
    zmq::socket_t* _cntrl;

    //misc
    std::string _endpoint;
    const std::string _hbport;
    const std::string _ioport;
    const std::string _shellport;
    const std::string _cntrlport;
    const std::string _key;
    const std::string _sig_scheme;

    std::thread _hbthread;
    std::thread _iothread;
};

#endif // ifndef juniper_juniper_juniper_H
