#include "cd.h"
#include "echo.h"
#include "exit.h"
#include "pwd.h"
#include "type.h"

#include <ctype.h>
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

int tokenize(char *input, char **tokens, int max_tokens) {
  if (!input || !tokens || max_tokens <= 0)
    return -1;

  int argc = 0;
  char *p = input;

// convenience macro to append into local buffer with overflow check
#define BUFSIZE 4096
#define APPEND_CHAR(buf, len, ch)                                              \
  do {                                                                         \
    if ((len) >= (int)sizeof(buf) - 1) {                                       \
      fprintf(stderr, "Token too long\n");                                     \
      return -1;                                                               \
    }                                                                          \
    (buf)[(len)++] = (ch);                                                     \
  } while (0)

  while (*p != '\0') {
    // skip leading whitespace (space or tab)
    while (*p == ' ' || *p == '\t')
      p++;
    if (*p == '\0')
      break;

    if (argc >= max_tokens - 1)
      break; // leave room for NULL terminator

    char buf[BUFSIZE];
    int len = 0;

    while (*p != '\0' && *p != ' ' && *p != '\t') {
      if (*p == '\'') {
        // single-quote mode: take everything literally until next single quote
        p++; // skip opening '
        while (*p != '\0' && *p != '\'') {
          APPEND_CHAR(buf, len, *p);
          p++;
        }
        if (*p == '\0') {
          fprintf(stderr, "Syntax error: unterminated single quote\n");
          return -1;
        }
        p++; // skip closing '
      } else if (*p == '"') {
        // double-quote mode: $, `, ", \ retain special meaning to the shell.
        // For our tokenizer: backslash in double quotes only escapes ", $, `,
        // \ and newline.
        p++; // skip opening "
        while (*p != '\0' && *p != '"') {
          if (*p == '\\') {
            // If backslash followed by newline: both are removed (line
            // continuation)
            if (*(p + 1) == '\n') {
              p += 2;
              continue;
            }
            char next = *(p + 1);
            if (next == '"' || next == '\\' || next == '$' || next == '`') {
              // remove backslash, keep next char
              p++; // skip backslash
              APPEND_CHAR(buf, len, *p);
              p++;
            } else if (next == '\0') {
              // backslash at end inside double quotes: syntax error
              fprintf(stderr,
                      "Syntax error: trailing backslash in double quote\n");
              return -1;
            } else {
              // POSIX: backslash before other chars is preserved as backslash +
              // char
              APPEND_CHAR(buf, len, '\\');
              p++; // move to next char which we'll emit normally in next loop
                   // iteration
            }
          } else {
            APPEND_CHAR(buf, len, *p);
            p++;
          }
        }
        if (*p == '\0') {
          fprintf(stderr, "Syntax error: unterminated double quote\n");
          return -1;
        }
        p++; // skip closing "
      } else if (*p == '\\') {
        // Outside quotes: backslash escapes the next character (including
        // newline -> continuation)
        if (*(p + 1) == '\0') {
          fprintf(stderr, "Syntax error: trailing backslash\n");
          return -1;
        }
        if (*(p + 1) == '\n') {
          // backslash-newline: remove both (line continuation)
          p += 2;
          continue;
        } else {
          // consume backslash and take next character literally
          p++; // skip backslash
          APPEND_CHAR(buf, len, *p);
          p++;
        }
      } else {
        // unquoted normal character
        APPEND_CHAR(buf, len, *p);
        p++;
      }
    }

    buf[len] = '\0';
    tokens[argc++] = strdup(buf);
    if (tokens[argc - 1] == NULL) {
      fprintf(stderr, "strdup failed\n");
      return -1;
    }
  }

  tokens[argc] = NULL;
#undef APPEND_CHAR
#undef BUFSIZE
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
