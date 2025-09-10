#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);
  while (1) {
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
    } else {
      printf("%s: command not found\n", ptr);
    }
  }
  return 0;
}
