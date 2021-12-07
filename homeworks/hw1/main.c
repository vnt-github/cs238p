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


/**
 ** @brief in case of redirections, or cd we need to shift the commands string to the left
 ** @param args Null terminated list of arguments (including program).
 ** @param from_i the index to begin shifting from
 ** @param shift_left distance from from_i to shift from
 ** @return Always returns 1, to continue execution.
 */
void shiftLeft(char **args, int from_i, int shift_len) {
  size_t i = from_i;
  while (args[i+shift_len]) {
    args[i] = args[i+shift_len];
    i++;
  }
  while (args[i]) {
    args[i] = NULL;
    i += 1;
  }
  // printf("shifted: %s", *args);

  return;
}


/**
 ** @brief set the input redirection.
 ** @param args Null terminated list of arguments (including program).
 */
void setInputRedirections(char **args) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], "<")) {
    // printf("%s\n", args[i]);
    i++;
  }
  if (args[i] && !strcmp(args[i], "<")) {
    i++;
    if (!args[i]) {
      fprintf(stderr, "input file name missing\n");
      return;
    }
    close(0);
    int ip = open(args[i], O_RDONLY);
    if (ip < 0) {
      perror("error opening input file\n");
    }
    // printf("input filename: %s %d\n", args[i], ip);
    shiftLeft(args, i-1, 2);
  }

  return;
}

/**
 ** @brief set the output redirection.
 ** @param args Null terminated list of arguments (including program).
 */
void setOutputRedirections(char **args) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], ">")) {
    // printf("%s\n", args[i]);
    i++;
  }
  if (args[i] && !strcmp(args[i], ">")) {
    i++;
    if (!args[i]) {
      fprintf(stderr, "output file name missing\n");
      return;
    }
    close(1);
    int ip = open(args[i], O_WRONLY|O_CREAT, 0644);
    if (ip < 0) {
      perror("error opening Output file");
    }
    // printf("output filename: %s %d\n", args[i], ip);
    shiftLeft(args, i-1, 2);
  }

  return;
}


/**
 ** @brief run a program via exec
 ** @param args Null terminated list of arguments (including program).
 ** @return Always returns 1, to continue execution.
 */
void run_command(char **args) {
  setInputRedirections(args);
  setOutputRedirections(args);

  if (execvp(args[0], args) == -1) {
      fprintf(stderr, "exec failed to execute: %s\n", args[0]);
      perror("exec failed");
  }
  exit(EXIT_FAILURE);
}

void run_pipe_commands(char **args) {
  int bufsize = 4*64;

  char** left = malloc(bufsize*sizeof(char *));

  size_t i = 0;
  size_t l_i = 0;
  while (args[i] && strcmp(args[i], "|")) {
    left[l_i] = strdup(args[i]);
    i++;
    l_i++;
  }
  left[l_i] = NULL;
  
  if (!args[i] || strcmp(args[i], "|") || !args[i+1]) {
    run_command(left);
    return;
  }

  i++;
  size_t r_i = 0;
  char** right = malloc(bufsize*sizeof(char *));
  while (args[i]) {
    right[r_i] = strdup(args[i]);
    i++;
    r_i++;
  }
  right[r_i] = NULL;

  int p[2];
  if (pipe(p) < 0) {
    perror("error creating pipe");
  }
  int pid_left = fork();
  if (pid_left < 0) {
    perror("pipe left fork failed");
    free(left);
    free(right);
    return;
  }
  if (pid_left == 0) {
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    run_command(left);
  }
  int pid_right = fork();
  if (pid_right < 0) {
    perror("pipe right fork failed");
    free(left);
    free(right);
    return;
  }
  if (pid_right == 0) {
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    run_pipe_commands(right);
  }
  close(p[0]);
  close(p[1]);
    int status, ret_pid;
    do {
      ret_pid = wait(&status);
      if (ret_pid == -1)
        perror("delivery of a signal to the calling process\n");
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  free(left);
  free(right);
  exit(EXIT_SUCCESS);
  return;
}

int main() {
    char *args[15];
    args[0] = "cat";
    args[1] = "<";
    args[2] = "foo.txt";
    args[3] = "|";
    args[4] = "grep";
    args[5] = "main";
    args[6] = "|";
    args[7] = "wc";
    args[8] = "-l";
    args[9] = ">";
    args[10] = "bar.txt"; 
    args[11] = NULL;
    run_pipe_commands(args);
    return 0;
}