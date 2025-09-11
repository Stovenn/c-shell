#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *builtins[] = {"echo", "exit", "type"};

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  while (true) {
    // Uncomment this block to pass the first stage
    printf("$ ");

    // Wait for user input
    char input[100];
    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }

    size_t inputLen = strlen(input);
    if (inputLen > 0 && input[inputLen - 1] == '\n') {
      //  removing trailing newline
      input[strlen(input) - 1] = '\0';
    }

    char *ptr = strtok(input, " ");
    if (ptr == NULL) {
      continue;
    }
    if (strcmp(ptr, "echo") == 0) {
      ptr = strtok(NULL, " ");
      while (ptr != NULL) {
        printf("%s ", ptr);
        ptr = strtok(NULL, " ");
      }
      printf("\n");
    } else if (strcmp(ptr, "exit") == 0) {
      ptr = strtok(NULL, " ");
      if (ptr == NULL || strcmp(ptr, "0") == 0) {
        break;
      } else {
        int code = atoi(ptr);
        return code;
      }
    } else if (strcmp(ptr, "type") == 0) {
      ptr = strtok(NULL, " ");
      if (ptr == NULL) {
        continue;
      }
      bool found = false;
      for (size_t i = 0; i < 3; ++i) {
        if (strcmp(ptr, builtins[i]) == 0) {
          printf("%s is a shell builtin\n", ptr);
          found = true;
          break;
        }
      }
      if (!found) {
        printf("%s: not found\n", ptr);
      }
    } else {
      printf("%s: command not found\n", ptr);
    }
  }
  return 0;
}
