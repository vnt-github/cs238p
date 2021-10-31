#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
int main() {
    char * message = "aaa\n";
    int pid = fork();
    if(pid != 0){
        char *echoargv[] = { "echo", "Hello\n", 0 };
        message = "bbb\n";
        execvp("echo", echoargv);
        printf("after execvp\n");
    }
    write(1, message, 4);
    exit(0);
}