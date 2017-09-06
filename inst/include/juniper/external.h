#ifndef juniper_juniper_external_H
#define juniper_juniper_external_H

#include <Rcpp.h>

typedef void(*finalizerT)(SEXP);
template<typename T>
SEXP createExternalPointer(T* p, finalizerT finalizer, const char* pname) {
  SEXP ptr;
  ptr = Rcpp::Shield<SEXP>(R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(ptr, finalizer, TRUE);
  return ptr;
}

#endif // juniper_juniper_external_H