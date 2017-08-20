#include <string>
#include <Rcpp.h>
#include <zmq.hpp>

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

static void zmqContextFinalizer(SEXP ctx) {
    zmq::context_t* context = reinterpret_cast<zmq::context_t*>(R_ExternalPtrAddr(ctx));
    if( context ) {
        delete context;
        R_ClearExternalPtr(ctx);
    }
}

// [[Rcpp::export]]
SEXP zmqContext() {
    zmq::context_t* zmqContext = new zmq::context_t(1);
    return createExternalPointer<zmq::context_t>(zmqContext, zmqContextFinalizer, "zmq::context_t*");
}
