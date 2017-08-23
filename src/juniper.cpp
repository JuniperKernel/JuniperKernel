#include <string>
#include <Rcpp.h>
#include <zmq.hpp>
#include <fstream>
#include <json.hpp>

#define LINGER 1000

#ifndef FINALIZERS_H
#define FINALIZERS_H
typedef void(*finalizerT)(SEXP);
template<typename T>
SEXP createExternalPointer(T* p, finalizerT finalizer, const char* pname) {
  SEXP _p;
  _p = Rcpp::Shield<SEXP>(R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(_p, finalizer, TRUE);
  return _p;
}
#endif // FINALIZERS_H

using nlohmann::json;

struct config {
  std::string control_port;
  std::string hb_port;
  std::string iopub_port;
  std::string ip;
  std::string key;
  std::string shell_port;
  std::string signature_scheme;
  std::string stdin_port;
  std::string transport;
};

// sample config:
//    { 
//      "control_port":54596
//    , "hb_port":54597
//    , "iopub_port":54594
//    , "ip":"127.0.0.1"
//    , "key":"8ac43620-2a4b-477a-8353-ea1eed1e41c2"
//    , "shell_port":54593
//    , "signature_scheme":"hmac-sha256"
//    , "stdin_port":54595
//    , "transport":"tcp"
//    }


class JuniperKernel {
  public:
    static JuniperKernel* make(const std::string connection_file) {
      std::ifstream ifs(connection_file);
      json connection_info = json::parse(ifs);
      config conf = {
        connection_info["control_port"].get<std::string>(),
        connection_info["hb_port"].get<std::string>(),
        connection_info["iopub_port"].get<std::string>(),
        connection_info["ip"].get<std::string>(),
        connection_info["key"].get<std::string>(),
        connection_info["shell_port"].get<std::string>(),
        connection_info["signature_scheme"].get<std::string>(),
        connection_info["stdin_port"].get<std::string>(),
        connection_info["transport"].get<std::string>(),
      };
      return new JuniperKernel(conf);
    }

    JuniperKernel(const config& conf)
      : _ctx(new zmq::context_t(1)),
        _shell(*_ctx, zmq::socket_type::router),
        _stdin(*_ctx, zmq::socket_type::router),
        _cntrl(*_ctx, zmq::socket_type::router),
        _iopub(*_ctx, zmq::socket_type::pub   ),
        _hbeat(*_ctx, zmq::socket_type::rep   )
    {
      char sep = (conf.transport=="tcp") ? ':' : '-';
      std::string endpoint = conf.transport + "://" + conf.ip + sep;
      
      // setup all of the sockets
      _shell.setsockopt(ZMQ_LINGER, LINGER);
      _shell.bind(endpoint + conf.shell_port);
      
      _stdin.setsockopt(ZMQ_LINGER, LINGER);
      _stdin.bind(endpoint + conf.stdin_port);
      
      _cntrl.setsockopt(ZMQ_LINGER, LINGER);
      _cntrl.bind(endpoint + conf.control_port);
      
      _iopub.setsockopt(ZMQ_LINGER, LINGER);
      _iopub.bind(endpoint + conf.iopub_port);
      
      _hbeat.setsockopt(ZMQ_LINGER, LINGER);
      _hbeat.bind(endpoint + conf.hb_port);
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
        _shell.setsockopt(LINGER, 0);
        _stdin.setsockopt(LINGER, 0);
        _cntrl.setsockopt(LINGER, 0);
        _iopub.setsockopt(LINGER, 0);
        _hbeat.setsockopt(LINGER, 0);
        delete _ctx;
      }
    }

  private:
    zmq::context_t* const _ctx;
    zmq::socket_t _shell;
    zmq::socket_t _stdin;
    zmq::socket_t _cntrl;
    zmq::socket_t _iopub;
    zmq::socket_t _hbeat;
};

// [[Rcpp::export]]
SEXP bootJuniper(const std::string connection_file) {
  JuniperKernel* jpro = JuniperKernel::make(connection_file);
  return createExternalPointer<JuniperKernel>(jpro, JuniperKernel::finalizer, "JuniperKernel*");
}
