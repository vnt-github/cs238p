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
      // printf("to execute: %s\n", args[0]);
      perror("execvp failed");
      exit(EXIT_FAILURE);
  }
}

void run_pipe_commands(char **args) {
  char** left = malloc(100*sizeof(char *));

  size_t i = 0;
  size_t l_i = 0;
  while (args[i] && strcmp(args[i], "|")) {
    left[l_i] = args[i];
    i++;
    l_i++;
  }
  
  if (!args[i] || strcmp(args[i], "|") || !args[i+1]) {
    run_command(left);
    return;
  }

  i++;
  size_t r_i = 0;
  char** right = malloc(100*sizeof(char *));
  while (args[i]) {
    right[r_i] = args[i];
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
    return;
  }
  if (pid_left == 0) {
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    run_command(left);
  } else {
    int pid_right = fork();
    if (pid_right < 0) {
      perror("pipe right fork failed");
      return;
    }
    if (pid_right == 0) {
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      run_pipe_commands(right);
    } else {
      close(p[0]);
      close(p[1]);
      int status;
      waitpid(pid_left, &status, WUNTRACED);
      waitpid(pid_right, &status, WUNTRACED);
      return;
    }
  }
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

    int pid, wait_p_id, status;
    if ((pid = fork()) < 0) {
        perror("fork call failed");
        return 0;
    } else if (pid == 0) {
      run(args);
    } else {
        do {
            wait_p_id = waitpid(pid, &status, WUNTRACED);
            if (wait_p_id < 0)
              perror("wait_p_id error");
            // printf("\nwait_p_id %d\n", wait_p_id);
        } while (!WIFEXITED(status) && WIFSIGNALED(status));
    }
    return 1;
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

  return sh_launch(args);   // launch
}


/**
 ** @brief Split a line into tokens (very naively).
 ** @param line The line.
 ** @return Null-terminated array of tokens.
 */
#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
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
        printf("238p$ ");
        line = sh_read_line();
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
