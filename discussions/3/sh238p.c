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

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
#define HISTORY_MAX_SIZE 256

char *history[HISTORY_MAX_SIZE];
unsigned history_count = 0;

void add_to_history(char *command)
{
  if (history_count < HISTORY_MAX_SIZE) {
    // strdup allocates memory for the new string on the heap.
    history[history_count++] = strdup(command);
  } else {
    free(history[0]);
    unsigned i=1;
    while (i < HISTORY_MAX_SIZE) {
      history[i - 1] = history[i];
      i++;
    }
    history[HISTORY_MAX_SIZE - 1] = strdup(command);
  }
}

int waitfor(int pid) {
  int ret_pid, status;
  do {
      ret_pid = waitpid(pid, &status, WUNTRACED);
      if (ret_pid == -1)
        perror("delivery of a signal to the calling process\n");
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  if (status) {
    fprintf(stderr, "pid %d did not exit normally, status: %d\n", pid, status);
  }
  return !status;
}

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

void setInputRedirections(char **args) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], "<")) {
    // printf("%s\n", args[i]);
    i++;
  }
  if (args[i] && !strcmp(args[i], "<")) {
    i++;
    if (!args[i]) {
      perror("input file name missing");
      return;
    }
    close(0);
    int ip = open(args[i], O_RDONLY);
    if (ip < 0) {
      perror("error opening input file");
    }
    // printf("input filename: %s %d\n", args[i], ip);
    shiftLeft(args, i-1, 2);
  }

  return;
}

void setOutputRedirections(char **args) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], ">")) {
    // printf("%s\n", args[i]);
    i++;
  }
  if (args[i] && !strcmp(args[i], ">")) {
    i++;
    if (!args[i]) {
      perror("output file name missing");
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

void run_command(char **args) {
  setInputRedirections(args);
  setOutputRedirections(args);
  if (execvp(args[0], args) == -1) {
      fprintf(stderr, "execvp failed to execute: %s\n", args[0]);
      perror("execvp failed");
  }
  exit(EXIT_FAILURE);
}

// TODO: make left and right realloc
void run_pipe_commands(char **args) {
  int bufsize = 4*SH_TOK_BUFSIZE;

  char** left = malloc(bufsize*sizeof(char *));

  size_t i = 0;
  size_t l_i = 0;
  while (args[i] && strcmp(args[i], "|")) {
    left[l_i] = strdup(args[i]);
    i++;
    l_i++;
  }
  
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
  if (!waitfor(pid_left)) {
    fprintf(stderr, "pid_left: %d did not exit normally\n", pid_left);
  }
  if (!waitfor(pid_right)) {
    fprintf(stderr, "pid_right: %d did not exit normally\n", pid_right);
  }

  free(left);
  free(right);
  exit(EXIT_SUCCESS);
  return;
}

void run(char **args) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], "|")) {
    // printf("%s\n", args[i]);
    i++;
  }

  if (args[i] && !strcmp(args[i], "|")) {
    if (!args[i-1]) {
      perror("missing left side of pipe");
    }
    if (!args[i+1]) {
      perror("missing right side of pipe");
    }
    run_pipe_commands(args);
  } else {
    run_command(args);
  }

}

/**
 ** @brief Launch a program and wait for it to terminate.
 ** @param args Null terminated list of arguments (including program).
 ** @return Always returns 1, to continue execution.
 */
int sh_launch(char **args){
    /* most of your code here */

    int pid;
    if ((pid = fork()) < 0) {
        perror("fork call failed");
        return 0;
    } else if (pid == 0) {
      // printf("child %d \n", (int) getpid());
      run(args);
    }
    return waitfor(pid);
}

int runBuildIn(char **args) {
  if (!strcmp(args[0], "cd")) {
    if (!args[1]) {
      fprintf(stderr, "cd: missing directory to move to");
    }
    if (chdir(args[1])) {
      perror("chdir failed");
      fprintf(stderr, "target dir: %s\n", args[1]);
    }
    return 1;
  } else if (!strcmp(args[0], "history")) {
    unsigned i = 0;
    while (i < history_count) {
      printf("\t %d %s", i+1, history[i]);
      i++;
    }
    return 1;
  }
  return 0;
}

/**
 ** @brief Execute shell built-in or launch program.
 ** @param args Null terminated list of arguments.
 ** @return 1 if the shell should continue running, 0 if it should terminate
 */
int sh_execute(char **args){
//   int i;
  if (args[0] == NULL) {
    return 1;  // An empty command was entered.
  }

  if (runBuildIn(args)) {
    return 1;
  }

  return sh_launch(args);   // launch
}


/**
 ** @brief Split a line into tokens (very naively).
 ** @param line The line.
 ** @return Null-terminated array of tokens.
 */

char **sh_split_line(char *line){
    int bufsize = SH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_backup;

    if(!tokens){
        fprintf(stderr, "sh238p: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += SH_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if(!tokens){
                free(tokens_backup);
                fprintf(stderr, "sh238p: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


/**
 ** @brief Read a line of input from stdin.
 ** @return The line from stdin.
 **/
char *sh_read_line(void){
    char *line = NULL;
    size_t bufsize = 0;  // have getline allocate a buffer for us
    if(getline(&line, &bufsize, stdin) == -1){
        if(feof(stdin)){ // We recieved an EOF
            exit(EXIT_SUCCESS);  
        }else{
            perror("sh238p: sh_read_line");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}


/**
 ** @brief Loop getting input and executing it.
 **/
void sh_loop(void){
    char *line;
    char **args;
    int status;
    do{
        // printf("%d 238p$ ", (int) getpid());
        printf("238p$ ");
        line = sh_read_line();
        add_to_history(line);
        args = sh_split_line(line);
        status = sh_execute(args);
        free(line);
        free(args);
    }while(status);
}


/**
 ** @brief Main entry point.
 ** @param argc Argument count.
 ** @param argv Argument vector.
 ** @return status code
 **/
int main(int argc, char **argv){
    sh_loop();
    return EXIT_SUCCESS;
}
