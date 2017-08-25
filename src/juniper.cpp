#include <string>
#include <thread>
#include <Rcpp.h>
#include <zmq.hpp>
#include <fstream>
#include <json.hpp>
#include <juniper.hpp>
#include <zmq_addon.hpp>

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
        _ioport(conf.iopub_port),
        _hbport(conf.hb_port),
        _key(conf.key),
        _sig(conf.signature_scheme),
        
        // these are the 3 incoming Jupyter channels
        _shell(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _stdin(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _cntrl(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
    
        // these are internal routing sockets that push messages (e.g. 
        // poison pills, results, etc.) to the heartbeat thread and 
        // iopub thread.
        _inproc_pub(new zmq::socket_t(*_ctx, zmq::socket_type::pub)),
        _inproc_sig(new zmq::socket_t(*_ctx, zmq::socket_type::pub))
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
    
    // Functional style polling with custom message handling
    // 
    // This is a lame poller as it expects only 2 items and
    // has a single handler (for the second item). The first
    // item is expected to be a signaler to suicide; while 
    // the second item receives Real Messages.
    //
    // would put this thing inside start_bg_threads, but
    // c++11 does not allow templated lambdas.
    template<typename OnMessage>
    void poller(zmq::pollitem_t (&items)[2], OnMessage handler) {

      while( 1 ) {
        zmq::poll(&items[0], 2, -1);
        if( /*poison pill*/ items[0].revents & ZMQ_POLLIN ) {
          // loop over sockets, setting their lingers to 0
          for(const zmq::pollitem_t item: items)
            ((zmq::socket_t*)item.socket)->setsockopt(ZMQ_LINGER, 0);
          break;
        }
        
        if( items[1].revents & ZMQ_POLLIN ) {
          handler();
        }
      }
    }
    
    // start the background threads
    // called as part of the kernel boot sequence
    void start_bg_threads() {
      // heartbeat thread
      std::thread hb([this]() {
        // bind to the heartbeat endpoint
        zmq::socket_t hb(*_ctx, zmq::socket_type::rep);
        init_socket(&hb, _endpoint + _hbport);

        zmq::socket_t sigsub = subscribe_to(inproc_sig);
        
        // setup pollitem_t instances and poll
        zmq::pollitem_t items[] = {
          {sigsub, 0, ZMQ_POLLIN, 0},
          {hb,     0, ZMQ_POLLIN, 0}
        };
        poller(items, [&hb]() {
          zmq::multipart_t msg;
          // ping-pong the message
          msg.recv(hb);
          msg.send(hb);
        });
      });

      // iopub thread
      std::thread io([this]() {
        // bind to the iopub endpoint
        zmq::socket_t io(*_ctx, zmq::socket_type::pub);
        init_socket(&io, _endpoint + _ioport);

        zmq::socket_t sigsub = subscribe_to(inproc_sig);
        zmq::socket_t pubsub = subscribe_to(inproc_pub);

        // setup pollitem_t instances and poll
        zmq::pollitem_t items[] = {
          {sigsub, 0, ZMQ_POLLIN, 0},
          {pubsub, 0, ZMQ_POLLIN, 0}
        };
        poller(items, [&pubsub, &io]() {
          // we've got some messages to send from the 
          // execution engine back to the client. Messages
          // from the executor are published to the inproc_pub
          // topic, and we forward them to the client here.
          zmq::multipart_t msg;
          msg.recv(pubsub);
          msg.send(io);
        });
      });
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
        _shell->setsockopt(ZMQ_LINGER, 0); delete _shell;
        _stdin->setsockopt(ZMQ_LINGER, 0); delete _stdin;
        _cntrl->setsockopt(ZMQ_LINGER, 0); delete _cntrl;
        delete _ctx;
      }
    }

  private:
    zmq::context_t* const _ctx;
    zmq::socket_t*  const _shell;
    zmq::socket_t*  const _stdin;
    zmq::socket_t*  const _cntrl;
    
    // inproc sockets
    zmq::socket_t* const _inproc_pub;
    zmq::socket_t* const _inproc_sig;
    const std::string inproc_pub = "inproc://pub";
    const std::string inproc_sig = "inproc://sig";

    std::string _endpoint;
    const std::string _hbport;
    const std::string _ioport;
    
    const std::string _key;
    const std::string _sig;
    
    zmq::socket_t subscribe_to(const std::string& inproc_topic) {
      // connect to the inproc signaller
      zmq::socket_t sub(*_ctx, zmq::socket_type::sub);
      // for option ZMQ_SUBSCRIBE, no need to set before connect (but we do anyways)
      // see: http://api.zeromq.org/4-1:zmq-setsockopt
      sub.setsockopt(ZMQ_SUBSCRIBE, "" /*no filter*/, 0);
      sub.connect(inproc_topic);
      return sub;
    }

    void init_socket(zmq::socket_t* socket, std::string endpoint) {
      socket->setsockopt(ZMQ_LINGER, LINGER);
      socket->bind(endpoint);
    }
};

// [[Rcpp::export]]
SEXP init_kernel(const std::string connection_file) {
  JuniperKernel* jpro = JuniperKernel::make(connection_file);
  return make_externalptr<JuniperKernel>(jpro, JuniperKernel::finalizer, "JuniperKernel*");
}

// [[Rcpp::export]]
SEXP boot_kernel(SEXP kernel) {
  JuniperKernel* jp = reinterpret_cast<JuniperKernel*>(R_ExternalPtrAddr(kernel));
  jp->start_bg_threads();
  while( 1 ) {
  }
}


// http://zguide.zeromq.org/page:all#Handling-Interrupt-Signals
// http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ