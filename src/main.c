#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int echo_fn(int argc, char **argv);
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

int exit_fn(int argc, char **argv);
int exit_fn(int argc, char **argv) {
  if (argc == 1) {
    return 0;
  }
  int code = atoi(argv[1]);
  return code;
}

const char *builtins_list[] = {"echo", "exit", "type"};

int type_fn(int argc, char **argv);
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

struct builtin {
  char *name;
  int (*func)(int argc, char **argv);
};

struct builtin builtins[] = {
    {"echo", echo_fn},
    {"exit", exit_fn},
    {"type", type_fn},
};

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (true) {
    // Uncomment this block to pass the first stage
    printf("$ ");

    // Wait for user input
    char input[255];
    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }

    size_t inputLen = strlen(input);
    if (inputLen > 0 && input[inputLen - 1] == '\n') {
      //  removing trailing newline
      input[strlen(input) - 1] = '\0';
    }

    char *tokens[64];
    int argCount = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
      tokens[argCount++] = token;
      token = strtok(NULL, " ");
    }
    tokens[argCount] = NULL;
    if (argCount == 0) {
      continue; // empty input
    }

    bool found = false;
    for (size_t i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
      if (strcmp(tokens[0], builtins[i].name) == 0) {
        int res = builtins[i].func(argCount, tokens);
        if (strcmp(tokens[0], "exit") == 0) {
          return res;
        }
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
        snprintf(fullpath, sizeof(fullpath), "%s/%s", token, tokens[0]);
        if (stat(fullpath, &sb) == 0 && sb.st_mode & S_IXUSR) {
          pid_t pid = fork();
          if (pid < 0) {
            perror("fork failed");
            return 1;
          } else if (pid == 0) {
            execv(fullpath, tokens);
          } else {
            wait(NULL);
          }
          found = true;
          break;
        }
        token = strtok(NULL, ":");
      }
	free(copy);
    }
    if (!found) {
      printf("%s: command not found\n", tokens[0]);
    }
  }
  return 0;
}
