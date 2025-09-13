#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int echo_fn(int argc, char **argv);
int echo_fn(int argc, char **argv)
{
  for (size_t i = 1; i < argc; i++){
    printf("%s", argv[i]);
    if (i < argc ) {
      printf(" ");
    }
  }
  printf("\n");
  return 0;
}

struct builtin
{
  char *name;
  int (*func)(int argc, char **argv);
};

struct builtin builtins[] = {
    {"echo", echo_fn},
};

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  while (true)
  {
    // Uncomment this block to pass the first stage
    printf("$ ");

    // Wait for user input
    char input[255];
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
      break;
    }

    size_t inputLen = strlen(input);
    if (inputLen > 0 && input[inputLen - 1] == '\n')
    {
      //  removing trailing newline
      input[strlen(input) - 1] = '\0';
    }

    char *tokens[64];
    int argCount = 0;
    char *token = strtok(input, " ");
    while (token != NULL)
    {
      tokens[argCount++] = token;
      token = strtok(NULL, " ");
    }
    tokens[argCount] = NULL;
    if (argCount == 0)
    {
      continue; // empty input
    }

    
    bool found = false;
    for (size_t i = 0; i < sizeof(builtins)/sizeof(builtins[0]); i++)
    {
      if (strcmp(tokens[0], builtins[i].name) == 0)
      {
        builtins[i].func(argCount, tokens);
        found = true;
        break;
      }
    }
    if (!found)
    {
      printf("%s: command not found\n", tokens[0]);
    }

    // char *ptr = strtok(input, " ");
    // if (ptr == NULL) {
    //   continue;
    // }
    // for (size_t i = 0; i < 3; ++i) {
    //   if (strcmp(ptr, builtins[i].name)) {
    //     int argsCount;
    //     char *argsVals;

    //     ptr = strtok(NULL, " ");
    //     while (ptr != NULL) {
    //       // add value then increment argsCount

    //       argsCount++;
    //     }
    //     int returnCode = builtins[i].func(argsCount, &argsVals);
    //     if (returnCode < 0) {
    //       return returnCode;
    //     }
    //     break;
    //   }
    //   if (strcmp(ptr, "echo") == 0) {
    //     ptr = strtok(NULL, " ");
    //     while (ptr != NULL) {
    //       printf("%s ", ptr);
    //       ptr = strtok(NULL, " ");
    //     }
    //     printf("\n");
    //   } else if (strcmp(ptr, "exit") == 0) {
    //     ptr = strtok(NULL, " ");
    //     if (ptr == NULL || strcmp(ptr, "0") == 0) {
    //       break;
    //     } else {
    //       int code = atoi(ptr);
    //       return code;
    //     }
    //   } else if (strcmp(ptr, "type") == 0) {
    //     ptr = strtok(NULL, " ");
    //     if (ptr == NULL) {
    //       continue;
    //     }
    //     bool found = false;
    //     for (size_t i = 0; i < 3; ++i) {
    //       if (strcmp(ptr, builtinsList[i]) == 0) {
    //         printf("%s is a shell builtin\n", ptr);
    //         found = true;
    //         break;
    //       }
    //     }
    //     if (!found) {
    //       printf("%s: not found\n", ptr);
    //     }
    //   } else {
    //     printf("%s: command not found\n", ptr);
    //   }
  }
  return 0;
}
