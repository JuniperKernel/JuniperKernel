#include <RInside.h>

int main(int argc, char* argv[]) {
  RInside R(argc, argv, true);
  R.repl();
  Rcpp::Rcout << "bye" << std::endl;
  exit(0);
}
