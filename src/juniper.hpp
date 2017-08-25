#include <string>

#define LINGER 1000

#ifndef FINALIZERS_H
#define FINALIZERS_H
typedef void(*finalizerT)(SEXP);
template<typename T>
SEXP make_externalptr(T* p, finalizerT finalizer, const char* pname) {
  SEXP kernel;
  kernel = Rcpp::Shield<SEXP>(R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(kernel, finalizer, TRUE);
  return kernel;
}
#endif // FINALIZERS_H


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