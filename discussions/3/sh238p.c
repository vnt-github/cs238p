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
#include <glob.h>

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
#define HISTORY_MAX_SIZE 256

char *history[HISTORY_MAX_SIZE];
unsigned history_count = 0;

/**
 ** @brief adds current command to history
 ** @param command current command to be executed
 */
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

/**
 ** @brief wait for program with pid terminate unless it's a bg task with &.
 ** @return Always returns 1, to continue execution.
 */
int waitfor(int pid, int is_bg) {
  int ret_pid, status;
  if (is_bg)
    return 1;
  
  // waiting in loop to make sure that we get status and log it.
  do {
      ret_pid = waitpid(pid, &status, WUNTRACED);
      if (ret_pid == -1)
        perror("delivery of a signal to the calling process\n");
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  if (status) {
    fprintf(stderr, "pid %d did not exit normally, status: %d\n", pid, status);
  }
  return 1;
}

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
 ** @brief change the current working directory
 ** this need to be executed by parent and seprate from run build in as else child will change directory
 ** @param args Null terminated list of arguments (including program).
 ** @return 0 if any error in changing to any further directory so that we can exit early, else 1
 */
int run_chdir(char **args) {
 if (args[0] && !strcmp(args[0], "cd")) {
    if (!args[1]) {
      // like in shell if cd has no argument then nothing happens.
      return 1;
    }
    if (chdir(args[1])) {
      perror("chdir failed");
      fprintf(stderr, "target dir: %s\n", args[1]);
      return 0;
    }
    // in case there are command after cd then we execute them normally by removing cd.
    shiftLeft(args, 0, 2);
  }
  return 1;
}

/**
 ** @brief handle running build ins like history etc
 ** we run these similar to other processes so that we can use redirections and piping with them too. 
 ** @param args Null terminated list of arguments (including program).
 ** @return return 1 when found and executed a build in else 0.
 */
int run_build_in(char **args) {
  if (args[0] && !strcmp(args[0], "history")) {
    unsigned i = 0;
    while (i < history_count) {
      fprintf(stdout, "\t %d %s", i+1, history[i]);
      i++;
    }
    return 1;
  }
  return 0;
}

/**
 ** @brief run a program via execvp
 ** @param args Null terminated list of arguments (including program).
 ** @return Always returns 1, to continue execution.
 */
void run_command(char **args) {
  setInputRedirections(args);
  setOutputRedirections(args);

  if (run_build_in(args)) {
    exit(EXIT_SUCCESS);
  }

  if (execvp(args[0], args) == -1) {
      fprintf(stderr, "execvp failed to execute: %s\n", args[0]);
      perror("execvp failed");
  }
  exit(EXIT_FAILURE);
}

void run_command_with_glob(char **args) {
  size_t i = 0;
  while (args[i] && strchr(args[i], '*') == NULL && strchr(args[i], '?') == NULL) {
    i++;
  }
  if (args[i] && (strchr(args[i], '*') != NULL || strchr(args[i], '?') != NULL )) {
    glob_t paths;
    int retval;
    paths.gl_offs = i;
    retval = glob(args[i], GLOB_DOOFFS, NULL, &paths);
    if (retval) {
      fprintf(stderr, "globbing failed for %s\n", args[i]);
      perror("globbing failed\n");
      return;
    }
    size_t j = 0;
    while (j < i) {
      paths.gl_pathv[0] = args[j];
      j++;
    }
    run_command(paths.gl_pathv);
  } else {
    run_command(args);
    return;
  }
}



// TODO: make left and right realloc
/**
 ** @brief run pipe commands via recurssion ie. with first command as left and rest as recursive right.
 ** @param args Null terminated list of arguments (including program).
 ** @param is_bg is the current command is to be run in background, so that we don't wait for it.
 */
void run_pipe_commands(char **args, int is_bg) {
  int bufsize = 4*SH_TOK_BUFSIZE;

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
    run_command_with_glob(left);
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
    run_command_with_glob(left);
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
    run_pipe_commands(right, is_bg);
  }
  close(p[0]);
  close(p[1]);
  if (!waitfor(pid_left, is_bg)) {
    fprintf(stderr, "pid_left: %d did not exit normally\n", pid_left);
  }
  if (!waitfor(pid_right, is_bg)) {
    fprintf(stderr, "pid_right: %d did not exit normally\n", pid_right);
  }

  free(left);
  free(right);
  exit(EXIT_SUCCESS);
  return;
}

/**
 ** @brief run the current command based on if it's a pipe or not.
 ** @param args Null terminated list of arguments (including program).
 ** @param is_bg is the current command is to be run in background, so that we don't wait for it.
 */
void run(char **args, int is_bg) {
  size_t i = 0;
  while (args[i] && strcmp(args[i], "|")) {
    // printf("%s\n", args[i]);
    i++;
  }

  if (args[i] && !strcmp(args[i], "|")) {
    if (!args[i-1]) {
      fprintf(stderr, "missing left side of pipe\n");
    }
    if (!args[i+1]) {
      fprintf(stderr, "missing right side of pipe\n");
    }
    run_pipe_commands(args, is_bg);
  } else {
    run_command_with_glob(args);
  }

}

/**
 ** @brief Launch a program and wait for it to terminate.
 ** @param args Null terminated list of arguments (including program).
 ** @return Always returns 1, to continue execution.
 */
int sh_launch(char **args){
    /* most of your code here */
  size_t i = 0;
  int is_bg = 0;
  while (args[i] && strcmp(args[i], "&")) {
    i++;
  }
  if (args[i] && !strcmp(args[i], "&")) {
    is_bg = 1;
    args[i] = NULL;
  }

  // NOTE: chdir has to be called in parent else forked child will get it's directory changed instead.
  if (!run_chdir(args)) {
    fprintf(stderr, "cd failed may be missing argument\n");
    return 1;
  }
  
  if (!args[0])
    return 1;

  int pid;
  if ((pid = fork()) < 0) {
      perror("fork call failed");
      return 0;
  } else if (pid == 0) {
    // printf("child %d \n", (int) getpid());
    run(args, is_bg);
  }
  return waitfor(pid, is_bg);
}

/**
 ** @brief iterate over the args if it's a ; seprated command.
 ** @param args Null terminated list of arguments (including program).
 ** @param from_i the current iterator position.
 ** @param semicolon_split is used to populate one sub command from the ; seperated command.
 ** @return 1 if there is a semicolon to the right
 */
int split_semicolons(char **args, int* from_i, char **semicolon_split) {
  size_t insert_i = 0;

  while (args[*from_i] && strcmp(args[*from_i], ";")) {
    semicolon_split[insert_i] = strdup(args[*from_i]);
    insert_i++;
    (*from_i)++;
  }
  semicolon_split[insert_i] = NULL;

  if (args[*from_i] && !strcmp(args[*from_i], ";")) {
    (*from_i)++;
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

  int bufsize = 4*SH_TOK_BUFSIZE;
  char **semicolon_split_cmd = malloc(bufsize*sizeof(char **));
  int from_i = 0;
  int start_i = 0;
  while (split_semicolons(args, &from_i, semicolon_split_cmd)) {
    if (!sh_launch(semicolon_split_cmd)) {
      free(semicolon_split_cmd);
      return 0;
    }
    start_i = from_i;
  }

  free(semicolon_split_cmd);
  return sh_launch(&args[start_i]);   // launch
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
        if(feof(stdin)){ // We received an EOF
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


/* TEST CASES:
ls /home
rm -rf x.txt
echo "238P is cool" > x.txt
cat < x.txt
rm -rf redir.out
echo "hello from test.sh" | wc -c > redir.out
cat redir.out
ls | sort | uniq | wc -c
ls /home | sort | uniq | wc -l
rm -rf usernames.txt
ls /home > usernames.txt
rm -rf users_with_v.txt
cat < usernames.txt | sort | uniq | grep v > users_with_v.txt
echo usernames with v:
cat users_with_v.txt
cd /home/vbharot/cs238p/discussions/3/new_folder ; echo "current location" ; pwd
cd .. ; echo "current location" ; pwd
history | grep txt
history > his.txt
cat < his.txt
echo 1 ; echo 2 ; echo 3 ; echo all done
ls > lis.txt ; history > his.txt ; wc < lis.txt ; wc < his.txt ; history | grep echo ; echo done
sleep 5 &
echo "sleep is going in bg"
*/