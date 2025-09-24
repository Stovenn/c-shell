#include "cd.h"
#include "echo.h"
#include "exit.h"
#include "pwd.h"
#include "type.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

struct builtin {
  char *name;
  int (*func)(int argc, char **argv);
};

struct builtin builtins[] = {
    {"echo", echo_fn}, {"exit", exit_fn}, {"type", type_fn},
    {"pwd", pwd_fn},   {"cd", cd_fn},
};

int tokenize(char *input, char **tokens, int max_tokens);
int tokenize(char *input, char **tokens, int max_tokens) {
  int argc = 0;
  char *p = input;

  while (*p != '\0') {

    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\0')
      break;

    if (argc >= max_tokens - 1)
      break;
    // Allocate a buffer for the token (on stack here, could malloc if needed)
    static char buf[1024];
    int len = 0;

    while (*p != '\0' && *p != ' ' && *p != '\t') {
      if (*p == '\'') {
        // Inside single quotes
        p++; // skip opening '
        while (*p != '\0' && *p != '\'') {
          buf[len++] = *p++;
        }
        if (*p == '\0') {
          fprintf(stderr, "Syntax error: unterminated single quote\n");
          return -1;
        }
        p++; // skip closing '
      } else if (*p == '"') {
        // Inside single quotes
        p++; // skip opening "
        while (*p != '\0' && *p != '"') {
          buf[len++] = *p++;
        }
        if (*p == '\0') {
          fprintf(stderr, "Syntax error: unterminated double quote\n");
          return -1;
        }
        p++; // skip closing "
      } else {
        // Regular char
        buf[len++] = *p++;
      }
    }
    buf[len] = '\0';

    tokens[argc++] = strdup(buf); // strdup so it's persistent
  }

  tokens[argc] = NULL;
  return argc;
}

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
    int argCount = tokenize(input, tokens, 64);
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
    // free tokens before next iteration
    for (int i = 0; tokens[i] != NULL; i++) {
      free(tokens[i]);
    }
  }
  return 0;
}
