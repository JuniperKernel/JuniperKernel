#include <xeus/xkernel.hpp>
#include <juniper/juniper.h>
#include <RInside.h>
#include <Rcpp.h>

std::string get_filename(int argc, char* argv[]) {
  std::string res = "";
  for (int i = 0; i < argc; ++i) {
    if ((std::string(argv[i]) == "-f") && (i + 1 < argc)) {
      res = argv[i + 1];
      break;
    }
  }
  return res;
}

int main(int argc, char *argv[]) {
  RInside rin(argc, argv, true);
  rin.parseEvalQ("library(JuniperKernel)");  // put the package on search path!
  std::string connection_file = get_filename(argc, argv);
  xeus::xconfiguration config = xeus::load_configuration(connection_file);
  using interpreter_ptr = std::unique_ptr<JadesInterpreter>;
  interpreter_ptr interpreter = interpreter_ptr(new JadesInterpreter(&rin));
  xeus::xkernel jk(config, "juniper_kernel", std::move(interpreter));
  jk.start();
  exit(0);
}
