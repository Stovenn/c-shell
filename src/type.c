#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char *builtins_list[] = {"echo", "exit", "type"};

int type_fn(int argc, char **argv) {
  if (argc != 2) {
    return 1;
  }

  bool found = false;
  for (size_t i = 0; i < sizeof(builtins_list) / sizeof(builtins_list[0]);
       ++i) {
    if (strcmp(argv[1], builtins_list[i]) == 0) {
      printf("%s is a shell builtin\n", argv[1]);
      found = true;
      break;
    }
  }
  if (!found) {
    // try to look in PATH
    const char *path = getenv("PATH");
    char *copy = strdup(path);
    char *token = strtok(copy, ":");
    while (token != NULL) {
      struct stat sb;
      char fullpath[PATH_MAX];
      snprintf(fullpath, sizeof(fullpath), "%s/%s", token, argv[1]);
      if (stat(fullpath, &sb) == 0 && sb.st_mode & S_IXUSR) {
        printf("%s is %s\n", argv[1], fullpath);
        found = true;
        break;
      }
      token = strtok(NULL, ":");
    }
    free(copy);
  }

  if (!found) {
    printf("%s not found\n", argv[1]);
    return 1;
  }
  return 0;
}
