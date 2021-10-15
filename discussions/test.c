#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(){
    if(fork()){
        close(0);
        open("foo.txt",O_CREAT|O_RDWR, S_IRWXU);
        printf("A");
    }else{
        close(1);
        open("bar.txt",O_CREAT|O_RDWR, S_IRWXU);
        printf("B");
    }
    return 0;
}