#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main () {
    int fds[2];
    char *bf;
    pipe(fds);
    if(!fork()){
        bf = malloc(14);
        // close(fds[1]);
        read(fds[0], bf, 13);
        // close(fds[0]);
    }else{
        // close(fds[0]);
        write(fds[1], "Hello, world.", 13);
        // close(fds[1]);
    }
    printf("%s\n", bf);
    return 0;
}