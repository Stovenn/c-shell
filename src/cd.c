#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int do_change_dir(const char *path){
  if (path == NULL) {
    return 1;
  }
  if (chdir(path) != 0) {
    printf("%s: No such file or directory\n", path);
    return 1;
  }
  return 0;
}

int cd_fn(int argc, char **argv) {
  if (argc < 2) {
    const char *path = getenv("HOME");
    if (path == NULL) {
      return 1;
    }
    return do_change_dir(path);
  }

  return do_change_dir(argv[1]);
}
