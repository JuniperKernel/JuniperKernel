#include <string>
#include <Rcpp.h>
#include <zmq.hpp>
#include <fstream>
#include <json.hpp>
#include <juniper.hpp>

using nlohmann::json;

class JuniperKernel {
  public:
    static JuniperKernel* make(const std::string connection_file) {
      std::ifstream ifs(connection_file);
      json connection_info = json::parse(ifs);
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
        _shell(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _stdin(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _cntrl(new zmq::socket_t(*_ctx, zmq::socket_type::router)),
        _iopub(new zmq::socket_t(*_ctx, zmq::socket_type::pub   )),
        _hbeat(new zmq::socket_t(*_ctx, zmq::socket_type::rep   ))
    {
      char sep = (conf.transport=="tcp") ? ':' : '-';
      std::string endpoint = conf.transport + "://" + conf.ip + sep;
      
      // socet setup
      _shell->setsockopt(ZMQ_LINGER, LINGER);
      _shell->bind(endpoint + conf.shell_port);
      
      _stdin->setsockopt(ZMQ_LINGER, LINGER);
      _stdin->bind(endpoint + conf.stdin_port);
      
      _cntrl->setsockopt(ZMQ_LINGER, LINGER);
      _cntrl->bind(endpoint + conf.control_port);
      
      _iopub->setsockopt(ZMQ_LINGER, LINGER);
      _iopub->bind(endpoint + conf.iopub_port);
      
      _hbeat->setsockopt(ZMQ_LINGER, LINGER);
      _hbeat->bind(endpoint + conf.hb_port);
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
        _iopub->setsockopt(ZMQ_LINGER, 0); delete _iopub;
        _hbeat->setsockopt(ZMQ_LINGER, 0); delete _hbeat;
        delete _ctx;
      }
    }

  private:
    zmq::context_t* const _ctx;
    zmq::socket_t* const _shell;
    zmq::socket_t* const _stdin;
    zmq::socket_t* const _cntrl;
    zmq::socket_t* const _iopub;
    zmq::socket_t* const _hbeat;
};

// [[Rcpp::export]]
SEXP bootJuniper(const std::string connection_file) {
  JuniperKernel* jpro = JuniperKernel::make(connection_file);
  return createExternalPointer<JuniperKernel>(jpro, JuniperKernel::finalizer, "JuniperKernel*");
}
