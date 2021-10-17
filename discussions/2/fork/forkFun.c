#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

static int count = 0;

int main(){
	count += 1;
	printf("%d\n", getpid());
	// fork();
	fork();
	fork();
	fork();
	printf("PID is=%d Hello :) \n", getpid());
	// printf("count %d", count);
	return 0;
}
