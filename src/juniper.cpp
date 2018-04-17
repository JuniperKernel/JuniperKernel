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
#include <zmq.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <xeus/xjson.hpp>
#include <xeus/xkernel.hpp>
#include <juniper/juniper.h>
#include <juniper/utils.h>
#include <juniper/gdevice.h>
#include <Rcpp.h>


// [[Rcpp::export]]
void boot_kernel(const std::string& connection_file) {
  xeus::xconfiguration config = xeus::load_configuration(connection_file);
  using interpreter_ptr = std::unique_ptr<JadesInterpreter>;
  interpreter_ptr interpreter = interpreter_ptr(new JadesInterpreter());
  xeus::xkernel jk(config, "juniper_kernel", std::move(interpreter));
  jk.start();
}

// [[Rcpp::export]]
void jk_device(std::string bg, double width, double height, double pointsize, bool standalone, Rcpp::List aliases) {
  makeDevice(bg, width, height, pointsize, standalone, aliases);
}

// [[Rcpp::export]]
void publish_execute_result(int execution_counter, std::string data) {
  xjson pub_data = xjson::parse(data);
  xeus::get_interpreter().publish_execution_result(execution_counter, std::move(pub_data), xeus::xjson::object());
}


//// FOR TESTING
//
//// [[Rcpp::export]]
//SEXP run_client(int hbport, int ioport, int shport, int ctport, int inport) {
//  JupyterTestClient* jclient = new JupyterTestClient(hbport, ioport, shport, ctport, inport);
//  return createExternalPointer<JupyterTestClient>(jclient, testClientFinalizer, "JupyterTestClient*");
//}
//
//static JupyterTestClient* get_client(SEXP jtc) {
//  return reinterpret_cast<JupyterTestClient*>(R_ExternalPtrAddr(jtc));
//}
//
//// [[Rcpp::export]]
//void client_exec_request(SEXP jtc, std::string payload) {
//  return get_client(jtc)->_shell.execute_request(payload);
//}
//
//// [[Rcpp::export]]
//std::string client_exec_reply(SEXP jtc) {
//  return get_client(jtc)->_shell.execute_reply();
//}
//
//// [[Rcpp::export]]
//void wait_for_hb(SEXP jtc) {
//  return get_client(jtc)->wait_for_hb();
//}
//
//// [[Rcpp::export]]
//std::string iopub_recv(SEXP jtc) {
//  return get_client(jtc)->_iomsg.recv();
//}