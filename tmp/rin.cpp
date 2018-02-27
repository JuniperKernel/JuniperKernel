#include <RInside.h>                    // for the embedded R via RInside

int main(int argc, char *argv[]) {
  RInside* R = new RInside(argc, argv);
  R->parseEvalQ("print('asdf')");
  exit(0);
}
