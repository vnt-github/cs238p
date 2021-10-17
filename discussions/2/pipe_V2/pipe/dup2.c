#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(){
 int fds[2];
 pid_t pid;
 
 /*create a pipe. file descriptors for the two ends of the pipe are placed in fds. */
 pipe(fds);
 
 /*fork a child process. */
 
 pid = fork();
 
 if (pid == (pid_t)0){
 	/*this is the child process. close our copy of the write end of the file descriptor. */
 	close(fds[1]);
 	/*connect the end read of the pipe to the standard input. */
 	dup2(fds[0], STDIN_FILENO);
 	
 	/*replace the child process with the 'sort' program*/
 	execlp("sort","sort",(char *)0);	
 }
 
 else{
  /*this is the parent process. */
  FILE* stream;
  /*close our copy of the read end of the file descriptor. */
  
  close(fds[0]);
  
  /*convert the write file descriptor to a FILE object, and write to it. */
  
  stream= fdopen(fds[1], "w");
  fprintf(stream,"this is the test. \n");
  fprintf(stream,"hello world. \n");
  fprintf(stream,"this is a sample test. \n");
  fprintf(stream,"C programming. \n");
  fflush(stream);
  close(fds[1]);
  /*wait for the child process to finish*/
  waitpid(pid, NULL, 0);
  
  
 
 }
 return 0;
}
