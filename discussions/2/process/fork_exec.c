#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>


/*spawn a child process running a new program. PROGRAM is the name of the program to run; the path will be searched for this program. ARG_LIST is a NULL-terminated list of character strings to be passed as the program's argument list. Returns the process ID of the spawned process. */

int spawn(char* program, char** arg_list){
 pid_t child_pid;
 /*duplicate this process. */
  child_pid = fork();
  
  if (child_pid!=0){
  /*this is the parent process */
  return child_pid;
  }
  else{
  /*now execute PROGRAM, searching for it in the path*/
  
  execvp(program, arg_list);
  /*the execvp function returns only if an error occurs. */
  fprintf(stderr, "an error occurred in execvp \n");
  abort();
  }
  
}

int main(){
/*the argument list to pass to the 'ls' command. */
char* arg_list[] = {
"ls", /* argv[0], the name of the program. */
 "-l",
 "/",
 NULL /*the argument list must end with a NULL. */

};

/*spawn a child process running the 'ls' command. Ignore the returned child process ID. */

spawn("ls", arg_list);
printf("done with the main program. \n");

return 0;

}
