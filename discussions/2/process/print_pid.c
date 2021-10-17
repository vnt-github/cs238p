#include <stdio.h>
#include <unistd.h>

int main(){
printf("The procecss ID is %d \n", (int)getpid());
printf("The parent procecss ID is %d \n", (int)getppid());

}
