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
#include <xeus/xjson.hpp>
#include <xeus/xserver.hpp>
#include <jades/jades.h>
#include <zmq.h>
#include <zmq.hpp>
#include <Rcpp.h>

static short interrupted=false;
static void sig_handler(int sig) { interrupted=true; }
static void sig_catcher(void) {
#ifdef _WIN32
  // TODO
#else
  struct sigaction action;
  action.sa_handler = sig_handler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
#endif
}

// init static vars now
std::atomic<long long> JMessage::_ctr{0};

static void kernelFinalizer(SEXP jk) {
  JuniperKernel* jkernel = reinterpret_cast<JadesKernel*>(R_ExternalPtrAddr(jk));
  if( jkernel ) {
    delete jkernel;
    R_ClearExternalPtr(jk);
  }
}

static JadesKernel* get_kernel(SEXP kernel) {
  return reinterpret_cast<JadesKernel*>(R_ExternalPtrAddr(kernel));
}

// [[Rcpp::export]]
SEXP init_kernel(const std::string& connection_file) {
  sig_catcher();

  xeus::xconfiguration config = xeus::load_configuration(connection_file);

  using interpreter_ptr = std::unique_ptr<JadesInterpreter>;
  zmq::context_t* ctx = new zmq::context_t(1);
  interpreter_ptr interpreter = interpreter_ptr(new JadesInterpreter(ctx));
  JadesKernel jk(config, "jades_kernel", std::move(interpreter), make_server, *ctx);

  // even if boot_kernel is exceptional and we don't run delete jk
  // this finalizer will be run on R's exit and a cleanup will trigger then
  // if the poller's never get a signal, then deletion will be blocked on join
  // until a forced shutdown comes in from a jupyter client
  return createExternalPointer<JuniperKernel>(jk, kernelFinalizer, "JadesKernel*");
}

// [[Rcpp::export]]
void boot_kernel(SEXP kernel) {
  JadesKernel* jk = get_kernel(kernel);
  jk->start();
  delete jk;
}

// [[Rcpp::export]]
void jk_device(SEXP kernel, std::string bg, double width, double height, double pointsize, bool standalone, Rcpp::List aliases) {
  makeDevice(get_kernel(kernel), bg, width, height, pointsize, standalone, aliases);
}

// [[Rcpp::export]]
void publish_execute_result(int execution_counter, Rcpp::List data) {
  xjson pub_data = from_list_r(data);
  publish_execution_result(execution_counter, std::move(pub_data), xjson());
}