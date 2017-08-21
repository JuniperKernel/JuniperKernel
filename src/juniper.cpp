#include <string>
#include <Rcpp.h>
#include <zmq.hpp>
#include <fstream>
#include <json.hpp>

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

class JuniperKernel {
  public:
    static JuniperKernel* make(const std::string &connection_file) {
      std::ifstream ifs(connection_file);
      json conn_info = json::parse(ifs);

      JuniperKernel* j = new JuniperKernel();
      zmq::context_t* ctx = new zmq::context_t(1);

      j->ctx=ctx;
      return j;
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
      if( ctx ) {
        delete ctx;
      }
    }

  private:
    zmq::context_t* ctx;

//    { "control_port":54596
//    , "hb_port":54597
//    , "iopub_port":54594
//    , "ip":"127.0.0.1"
//    , "key":"8ac43620-2a4b-477a-8353-ea1eed1e41c2"
//    , "shell_port":54593
//    , "signature_scheme":"hmac-sha256"
//    , "stdin_port":54595
//    , "transport":"tcp"
//    }
};

// [[Rcpp::export]]
SEXP bootJuniper(const std::string connection_file) {
  JuniperKernel* jpro = JuniperKernel::make(connection_file);
  return createExternalPointer<JuniperKernel>(jpro, JuniperKernel::finalizer, "JuniperKernel*");
}
