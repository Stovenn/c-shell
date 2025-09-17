#include <stdlib.h>

int exit_fn(int argc, char **argv) {
  if (argc == 1) {
    return 0;
  }
  int code = atoi(argv[1]);
 return code;
}
