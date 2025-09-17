#include <stdio.h>

int echo_fn(int argc, char **argv) {
  for (size_t i = 1; i < argc; i++) {
    printf("%s", argv[i]);
    if (i < argc) {
      printf(" ");
    }
  }
  printf("\n");
  return 0;
}

