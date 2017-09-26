#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <json.hpp>
#include <juniper/conf.h>
#include <juniper/sockets.h>
#include <juniper/background.h>
#include <juniper/requests.h>
#include <juniper/external.h>
#include <juniper/gdevice.h>
#include <juniper/juniper.h>
#include <juniper/xbridge.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
void handler(int sig){}
#else
#include <execinfo.h>
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif

static void kernelFinalizer(SEXP jk) {
  JuniperKernel* jkernel = reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(jk));
  if( jkernel ) {
    delete jkernel;
    R_ClearExternalPtr(jk);
  }
}

static JuniperKernel* get_kernel(SEXP kernel) {
  return reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(kernel));
}

// setup the mocked xeus interpreter
static xmock* _xm;
xinterpreter& xeus::get_interpreter() { return *_xm; }

// [[Rcpp::export]]
SEXP init_kernel(const std::string& connection_file) {
  JuniperKernel* jk = JuniperKernel::make(connection_file);
  _xm->_jk=jk;  // mocked interpreter needs pointer to the kernel

  // even if boot_kernel is exceptional and we don't run delete jk
  // this finalizer will be run on R's exit and a cleanup will trigger then
  // if the poller's never get a signal, then deletion will be blocked on join
  // until a forced shutdown comes in from a jupyter client
  return createExternalPointer<JuniperKernel>(jk, kernelFinalizer, "JuniperKernel*");
}

// [[Rcpp::export]]
void boot_kernel(SEXP kernel) {
  signal(SIGSEGV, handler);
  JuniperKernel* jk = get_kernel(kernel);
  jk->start_bg_threads();
  jk->run();
  delete jk;
  delete _xm;
}

// [[Rcpp::export]]
void stream_stdout(SEXP kernel, const std::string& output) {
  get_kernel(kernel)->_request_server->stream_stdout(output);
}

// [[Rcpp::export]]
void stream_stderr(SEXP kernel, const std::string& err) {
  get_kernel(kernel)->_request_server->stream_stderr(err);
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
