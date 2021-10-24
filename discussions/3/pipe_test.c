/* pipe2.c */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

void run_command(char **args) {
  // setInputRedirections(args);
  // setOutputRedirections(args);
  if (execvp(args[0], args) == -1) {
      printf("to execute: %s\n", args[0]);
      perror("execvp failed");
      exit(EXIT_FAILURE);
  }
}

int main()
{
    char *args[4];
    args[0] = "ls";
    args[1] = "|";
    args[2] = "sort";
    args[3] = NULL;

    char** left = malloc(100*sizeof(char *));

    size_t i = 0;
    size_t l_i = 0;
    while (args[i] && strcmp(args[i], "|")) {
      left[l_i] = args[i];
      i++;
      l_i++;
    }
    i++;

    size_t r_i = 0;
    char** right = malloc(100*sizeof(char *));
    while (args[i]) {
      right[r_i] = args[i];
      i++;
      r_i++;
    }

    int pipefds[2];
    //create pipe
    if(pipe(pipefds) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int pid_left = fork();
    if (pid_left == 0) {
      printf(" left child write in pipe\n");
      close(1);
      dup(pipefds[1]);
      close(pipefds[0]);
      close(pipefds[1]);

      run_command(left); 
      wait(NULL);                    
    } 
    
    int pid_right = fork();
    if (pid_right == 0) {
      close(0);
      dup(pipefds[0]);
      close(pipefds[0]);
      close(pipefds[1]);
      
      run_command(right);
      printf("CHILD: EXITING!");
      exit(EXIT_SUCCESS);
    }
    return 0;
}