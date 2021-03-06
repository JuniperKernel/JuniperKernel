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
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <nlohmann/json.hpp>
#include <juniper/conf.h>
#include <zmq.h>
#include <zmq.hpp>
#include <juniper/gdevice.h>
#include <juniper/juniper.h>
#include <juniper/xbridge.h>
#include <juniper/external.h>
#include <jclient/jclient.h>
#include <Rcpp.h>


// init static vars now
std::atomic<long long> JMessage::_ctr{0};

static void kernelFinalizer(SEXP jk) {
  JuniperKernel* jkernel = reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(jk));
  if( jkernel ) {
    delete jkernel;
    R_ClearExternalPtr(jk);
  }
}

static void testClientFinalizer(SEXP jtc) {
  JupyterTestClient* jclient = reinterpret_cast<JupyterTestClient*>(R_ExternalPtrAddr(jtc));
  if( jclient ) {
    Rcpp::Rcout << "deleting test client" << std::endl;
    delete jclient;
    Rcpp::Rcout << "attempting to clear the external pointer" << std::endl;
    R_ClearExternalPtr(jtc);
    Rcpp::Rcout << "external pointer for test client cleared" << std::endl;
  }
}

static void xmockFinalizer(SEXP xm) {
  xmock* _xm = reinterpret_cast<xmock*>(R_ExternalPtrAddr(xm));
  if( _xm ) {
    delete _xm;
    R_ClearExternalPtr(xm);
  }
}

static JuniperKernel* get_kernel(SEXP kernel) {
  return reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(kernel));
}

xmock* _xm;
SEXP R_xm = nullptr;
// [[Rcpp::export]]
SEXP init_kernel(const std::string& connection_file) {
  JuniperKernel* jk = JuniperKernel::make(connection_file);

  _xm = new xmock();
  _xm->_jk=jk;  // mocked interpreter needs pointer to the kernel

  // even if boot_kernel is exceptional and we don't run delete jk
  // this finalizer will be run on R's exit and a cleanup will trigger then
  // if the poller's never get a signal, then deletion will be blocked on join
  // until a forced shutdown comes in from a jupyter client
  return createExternalPointer<JuniperKernel>(jk, kernelFinalizer, "JuniperKernel*");
}

// [[Rcpp::export]]
SEXP boot_kernel(SEXP kernel, int interrupt_event) {
  JuniperKernel* jk = get_kernel(kernel);
  return jk->start_bg_threads(interrupt_event);
}

//' The XMock
//'
//' Get the xeus mock interpreter for interoperability with other
//' projects.
//'
//' @author Spencer Aiello
//'
//' @export
// [[Rcpp::export]]
SEXP the_xmock() {
  if( _xm )
    return R_xm ? (R_xm = createExternalPointer<xmock>(_xm, xmockFinalizer, "xmock*")) : R_xm;
  Rcpp::stop("no xmock available.");
}

// [[Rcpp::export]]
SEXP sock_recv(SEXP kernel, std::string sockName) {
  JuniperKernel* jk = get_kernel(kernel);
  return jk->recv(sockName);
}

// [[Rcpp::export]]
void post_handle(SEXP kernel, Rcpp::List res, std::string sockName) {
  JuniperKernel* jk = get_kernel(kernel);
  jk->post_handle(res, sockName);
}

// [[Rcpp::export]]
void rebroadcast_input(SEXP kernel, const std::string& execution_input, const int execution_count) {
  get_kernel(kernel)->_request_server->rebroadcast_input(execution_input, execution_count);
}

// [[Rcpp::export]]
void execute_result(SEXP kernel, Rcpp::List data) {
  get_kernel(kernel)->_request_server->execute_result(from_list_r(data));
}

// [[Rcpp::export]]
void jk_device(SEXP kernel, std::string bg, double width, double height, double pointsize, bool standalone, Rcpp::List aliases) {
  makeDevice(get_kernel(kernel), bg, width, height, pointsize, standalone, aliases);
}

// [[Rcpp::export]]
SEXP filter_comms(std::string target_name) {
  json comms;
  const xmock& xm = get_xmock();
  for( auto it = xm.comm_manager().comms().cbegin(); it != xm.comm_manager().comms().cend(); ++it ) {
    const std::string& name = it->second->target().name();
    if( target_name.empty() || name == target_name ) {
      xjson info;
      info["target_name"] = name;
      comms[it->first] = std::move(info);
    }
  }
  if( comms.empty() ) comms = {};
  return from_json_r({{"status", "ok"}, {"comms", comms}});
}

// [[Rcpp::export]]
void comm_request(const std::string type) {
  xmock& xm = get_xmock();
  JMessage* jm = &xm._jk->_request_server->_cur_msg;
  xmessage xmsg(to_xmessage(jm->get(), jm->ids(), std::move(jm->bufs())));
  if( type=="open" ) xm.comm_manager().comm_open( xmsg);
  if( type=="close") xm.comm_manager().comm_close(xmsg);
  if( type=="msg"  ) xm.comm_manager().comm_msg(  xmsg); 
}




// FOR TESTING

// [[Rcpp::export]]
SEXP run_client(int hbport, int ioport, int shport, int ctport, int inport) {
  JupyterTestClient* jclient = new JupyterTestClient(hbport, ioport, shport, ctport, inport);
  return createExternalPointer<JupyterTestClient>(jclient, testClientFinalizer, "JupyterTestClient*");
}

static JupyterTestClient* get_client(SEXP jtc) {
  return reinterpret_cast<JupyterTestClient*>(R_ExternalPtrAddr(jtc));
}

// [[Rcpp::export]]
void client_exec_request(SEXP jtc, std::string payload) {
  return get_client(jtc)->_shell.execute_request(payload);
}

// [[Rcpp::export]]
std::string client_exec_reply(SEXP jtc) {
  return get_client(jtc)->_shell.execute_reply();
}

// [[Rcpp::export]]
void wait_for_hb(SEXP jtc) {
  return get_client(jtc)->wait_for_hb();
}

// [[Rcpp::export]]
std::string iopub_recv(SEXP jtc) {
  return get_client(jtc)->_iomsg.recv();
}
