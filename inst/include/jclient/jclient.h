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
#include <jclient/iopub.h>
#include <jclient/hb.h>
#include <jclient/shell.h>
#include <jclient/control.h>
#include <jclient/stdin.h>
#include <zmq.h>
#include <zmq.hpp>
#include <Rcpp.h>

class JupyterTestClient {
  public:
    const std::string _key = "cc496d37-59a9-4c61-8900-d826985f564d";
    zmq::context_t* _ctx;
//    Shell _shell;
//    Stdin _stdin;
//    Ctrl _ctrl;
    zmq::socket_t* _inproc_sig;
    HB _hb;
//    IOPub _iopub;


    JupyterTestClient()
      /*, _shell(_key,_ctx), _stdin(_ctx), _ctrl(_ctx)*/
//        Rcpp::Rcout << "initializing juniper test client" << std::endl;
//        _inproc_sig = listen_on(*_ctx, INPROC_SIG, zmq::socket_type::pub);
//        _hb.start_hb(_ctx);
//        _iopub.start_iopub(_ctx);
      {
        _ctx = new zmq::context_t(1);
       _inproc_sig = listen_on(*_ctx, INPROC_SIG, zmq::socket_type::pub);
        _hb.start_hb(_ctx);
      }

    ~JupyterTestClient() {
      // force a shutdown
//      _shell.~Shell();
//      _stdin.~Stdin();
//      _ctrl.~Ctrl();
      zmq::message_t m(0); _inproc_sig->send(m);
      _inproc_sig->setsockopt(ZMQ_LINGER,0);
      Rcpp::Rcout << "Awaiting hb and iopub threads..." << std::endl;
      _hb._hb_t.join();
      Rcpp::Rcout << "Heartbeat thread shutdown successfully" << std::endl;
//      _iopub._io_t.join();
      Rcpp::Rcout << "IOPub thread shutdown successfully" << std::endl;
      delete _inproc_sig;
      Rcpp::Rcout << "_inproc_sig deleted" << std::endl;
      if( _ctx )
        delete _ctx;
      Rcpp::Rcout << "Juniper Test Client successfully destroyed." << std::endl;
    }

    std::string config() {
      json conf;
//      conf["control_port"    ] = _ctrl._port;
//      conf["hb_port"         ] = _hb._port;
//      conf["iopub_port"      ] = _iopub._port;
      conf["ip"              ] = "127.0.0.1";
//      conf["key"             ] = _key;
//      conf["shell_port"      ] = _shell._port;
      conf["signature_scheme"] = "hmac-sha256";
//      conf["stdin_port"      ] = _stdin._port;
      conf["transport"       ] = "tcp";
      return conf.dump();
    }
};