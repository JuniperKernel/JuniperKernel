#include <RInside.h>

typedef void(*finalizerT)(SEXP);
template<typename T>
SEXP createExternalPointer(T* p, finalizerT finalizer, const char* pname) {
  SEXP ptr;
  ptr = Rcpp::Shield<SEXP>(R_MakeExternalPtr(reinterpret_cast<void*>(p),Rf_install(pname),R_NilValue));
  R_RegisterCFinalizerEx(ptr, finalizer, TRUE);
  return ptr;
}

static void rinsideFinalizer(SEXP rin) {
  RInside* rinside = reinterpret_cast<RInside*>(R_ExternalPtrAddr(rin));
  if( rinside ) {
    R_ClearExternalPtr(rin);
  }
}

int main(int argc, char* argv[]) {
  RInside R(argc, argv, true, true);
  SEXP rin = createExternalPointer<RInside>(&R, rinsideFinalizer, "RInside*");
  R.assign(rin, "__rin__");
  std::string boot = "JuniperKernel::bootKernel(";
  std::string conn = std::string(*(++argv));
  R.parseEvalQ(boot + "\"" + conn + "\"" + ")");
  exit(0);
}
