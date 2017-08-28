#ifndef juniper_juniper_juniper_H
#define juniper_juniper_juniper_H

#include <string>
#include <Rcpp.h>

#define LINGER 1000 // number of millis to linger for
#define INPROC_PUB "inproc://pub"
#define INPROC_SIG "inproc://sig"

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

  void print_conf() {
    Rcpp::Rcout << "{"                                                << std::endl;
    Rcpp::Rcout << "  control_port: "     << control_port      << "," << std::endl;
    Rcpp::Rcout << "  hb_port: "          << hb_port           << "," << std::endl;
    Rcpp::Rcout << "  iopub_port: "       << iopub_port        << "," << std::endl;
    Rcpp::Rcout << "  ip: "               << ip                << "," << std::endl;
    Rcpp::Rcout << "  key: "              << key               << "," << std::endl;
    Rcpp::Rcout << "  shell_port: "       << shell_port        << "," << std::endl;
    Rcpp::Rcout << "  signature_scheme: " << signature_scheme  << "," << std::endl;
    Rcpp::Rcout << "  stdin_port: "       << stdin_port        << "," << std::endl;
    Rcpp::Rcout << "  transport: "        << transport                << std::endl;
    Rcpp::Rcout << "}"                                                << std::endl;
  }
};

#endif // ifndef juniper_juniper_juniper_H
