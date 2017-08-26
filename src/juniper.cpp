#include <string>
#include <thread>
#include <Rcpp.h>
#include <zmq.hpp>
#include <fstream>
#include <json.hpp>
#include <juniper.hpp>
#include <zmq_addon.hpp>
#include <sockets.hpp>
#include <background.hpp>

class JuniperKernel {
  public:
    static JuniperKernel* make(const std::string connection_file) {
      std::ifstream ifs(connection_file);
      nlohmann::json connection_info = nlohmann::json::parse(ifs);
      config conf = {
        connection_info["control_port"    ].get<std::string>(),
        connection_info["hb_port"         ].get<std::string>(),
        connection_info["iopub_port"      ].get<std::string>(),
        connection_info["ip"              ].get<std::string>(),
        connection_info["key"             ].get<std::string>(),
        connection_info["shell_port"      ].get<std::string>(),
        connection_info["signature_scheme"].get<std::string>(),
        connection_info["stdin_port"      ].get<std::string>(),
        connection_info["transport"       ].get<std::string>(),
      };
      return new JuniperKernel(conf);
    }

    JuniperKernel(const config& conf)
      : _ctx(new zmq::context_t(1)),
        
        // these are the 3 incoming Jupyter channels
        _shell(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _stdin(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _cntrl(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
    
        // these are internal routing sockets that push messages (e.g. 
        // poison pills, results, etc.) to the heartbeat thread and 
        // iopub thread.
        _inproc_pub(new zmq::socket_t(*_ctx, zmq::socket_type::pub)),
        _inproc_sig(new zmq::socket_t(*_ctx, zmq::socket_type::pub)),

        _hbport(conf.hb_port),
        _ioport(conf.iopub_port),
        _key(conf.key),
        _sig(conf.signature_scheme)
    {
      char sep = (conf.transport=="tcp") ? ':' : '-';
      _endpoint = conf.transport + "://" + conf.ip + sep;

      // socket setup
      init_socket(_shell, _endpoint + conf.shell_port);
      init_socket(_stdin, _endpoint + conf.stdin_port);
      init_socket(_cntrl, _endpoint + conf.control_port);

      // iopub and hbeat get their own threads and we
      // communicate via the inproc topics sig/sub
      // these get bound in the main thread
      init_socket(_inproc_pub, inproc_pub);
      init_socket(_inproc_sig, inproc_sig);
    }
    
    // start the background threads
    // called as part of the kernel boot sequence
    void start_bg_threads() {
      start_hb_thread(*_ctx, _endpoint + _hbport, inproc_sig);
      start_io_thread(*_ctx, _endpoint + _ioport, inproc_sig, inproc_pub);
    }

    // runs in the main the thread, polls shell and controller
    void run() {
      zmq::socket_t sigsub = subscribe_to(*_ctx, inproc_sig);
      zmq::pollitem_t items[] = {
        {sigsub,  0, ZMQ_POLLIN, 0},
        {*_cntrl, 0, ZMQ_POLLIN, 0},
        {*_shell, 0, ZMQ_POLLIN, 0}
      };

      std::function<bool()> handlers[] = {
        [&sigsub]() {
          return false;
        },
        [this]() {
          zmq::multipart_t msg;
          msg.recv(*_cntrl);
          // special handling of control messages
          return true;
        },
        [this]() {
          zmq::multipart_t msg;
          msg.recv(*_shell);
          // special handling of shell messages
          return true;
        }
      };
      poller(items, handlers, 3);
    }

    static void finalizer(SEXP jpro) {
      JuniperKernel* jp = reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(jpro));
      if( jp ) {
        delete jp;
        R_ClearExternalPtr(jpro);
      }
    }

    ~JuniperKernel() {
      // set linger to 0 on all sockets
      // destroy sockets
      // destoy ctx
      if( _ctx ) {
        _cntrl->setsockopt(ZMQ_LINGER, 0); delete _cntrl;
        delete _ctx;
      }
    }

  private:
    // context is shared by all threads, cause there 
    // ain't no GIL to stop us now! ...we can build this thing together!
    zmq::context_t* const _ctx;
    
    
    // jupyter/zmq routers
    zmq::socket_t*  const _shell;
    zmq::socket_t*  const _stdin;
    zmq::socket_t*  const _cntrl;
    
    // inproc sockets
    zmq::socket_t* const _inproc_pub;
    zmq::socket_t* const _inproc_sig;
    const std::string inproc_pub = "inproc://pub";
    const std::string inproc_sig = "inproc://sig";

    //misc
    std::string _endpoint;
    const std::string _hbport;
    const std::string _ioport;
    
    const std::string _key;
    const std::string _sig;
};

// [[Rcpp::export]]
SEXP init_kernel(const std::string connection_file) {
  JuniperKernel* jpro = JuniperKernel::make(connection_file);
  return make_externalptr<JuniperKernel>(jpro, JuniperKernel::finalizer, "JuniperKernel*");
}

// [[Rcpp::export]]
void boot_kernel(SEXP kernel) {
  JuniperKernel* jp = reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(kernel));
  jp->start_bg_threads();
  jp->run();
}


// http://zguide.zeromq.org/page:all#Handling-Interrupt-Signals
// http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ