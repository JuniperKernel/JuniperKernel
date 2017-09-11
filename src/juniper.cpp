#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <Rcpp.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <json.hpp>
#include <juniper/conf.h>
#include <juniper/sockets.h>
#include <juniper/background.h>
#include <juniper/requests.h>
#include <juniper/external.h>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
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
      conf.print_conf();
      return new JuniperKernel(conf);
    }

    // start the background threads
    // called as part of the kernel boot sequence
    void start_bg_threads() {
      _hbthread = start_hb_thread(*_ctx, _endpoint + _hbport);
      _iothread = start_io_thread(*_ctx, _endpoint + _ioport);
    }

    // runs in the main the thread, polls shell and controller
     void run() const {
       zmq::socket_t* cntrl = listen_on(*_ctx, _endpoint + _cntrlport, zmq::socket_type::router);
       zmq::socket_t* shell = listen_on(*_ctx, _endpoint + _shellport, zmq::socket_type::router);
       const RequestServer& server = *_request_server;
       const std::string key = _key;
       std::function<bool()> handlers[] = {
         [&cntrl, &key, &server]() {
           zmq::multipart_t msg;
           msg.recv(*cntrl);
           server.serve(msg, *cntrl);
           return true;
         },
         [&shell, &key, &server]() {
           zmq::multipart_t msg;
           msg.recv(*shell);
           server.serve(msg, *shell);
           return true;
         }
       };
       poll(*_ctx, (zmq::socket_t* []){cntrl, shell}, handlers, 2);
     }

    ~JuniperKernel() {
      // set linger to 0 on all sockets; destroy sockets; finally destoy ctx
      _hbthread.join();
      _iothread.join();
      _Tthread.join();
      if( _request_server )
        delete _request_server;

      if( _ctx ) {
        _stdin     ->setsockopt(ZMQ_LINGER, 0); delete _stdin;
        delete _ctx;
      }
    }

  private:
    // context is shared by all threads, cause there 
    // ain't no GIL to stop us now! ...we can build this thing together!
    zmq::context_t* const _ctx;

    // jupyter stdin
    zmq::socket_t*  const _stdin;

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
    std::thread _Tthread;
};


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

// [[Rcpp::export]]
SEXP init_kernel(const std::string& connection_file) {
  JuniperKernel* jk = JuniperKernel::make(connection_file);
  return createExternalPointer<JuniperKernel>(jk, kernelFinalizer, "JuniperKernel*");
}

// [[Rcpp::export]]
void boot_kernel(SEXP kernel) {
  signal(SIGSEGV, handler);
  JuniperKernel* jk = get_kernel(kernel);
  jk->start_bg_threads();
  jk->run();
  delete jk;
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
void send_execute_result(SEXP kernel, Rcpp::List data) {
  json j = from_list_r(data);
  get_kernel(kernel)->_request_server->send_execute_result(j);
}

// http://zguide.zeromq.org/page:all#Handling-Interrupt-Signals
// http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ
