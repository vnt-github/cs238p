#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* write count copies of message to stream, pausing for a second between each.*/

void writer (const char* message, int count, FILE* stream){
 for(; count >0; --count){
  /*write the message to the stream, and send it off immediately.*/
  
  fprintf(stream, "%s\n", message);
  fflush(stream);
  /*snooze a while.*/
  sleep(1);
 }
}

/*read random strings from the stream as long as possible.*/

void reader(FILE* stream){
 char buffer[1024];
 /*read until we hit the end of the stream. fgets reads until either a newline or the EOF*/
 
 while(!feof(stream) && !ferror(stream) && fgets(buffer, sizeof(buffer), stream)!=NULL)
 fputs(buffer, stdout);
 }
 int main(){
  int fds[2];
  pid_t pid;
  /*create a piple. File descriptors for the two ends of the pipe are placed in fds. */
   pipe(fds);
   /*fork a chil process */
   
   pid=fork();
   
   if(pid == (pid_t) 0){
    FILE* stream;
    /*this is the child process. close our copy of the writer end of the file descriptor. */
    
    close(fds[1]);
    /* convert the read file descriptor to a File object, and read from 	it.  */
    
    stream = fdopen(fds[0], "r");
    reader(stream);
    close(fds[0]);
    }
    
    else{
    /*this is the parent process. */
    FILE* stream;
    /*close our copy of the read end of the file descriptor*/
    close(fds[0]);
    /*convert the write file descriptor to a File object, and write to it. */
    stream = fdopen(fds[1], "w");
    writer("Hello world.", 5, stream);
    close(fds[1]);
    }
 	return 0;   
    }
 
