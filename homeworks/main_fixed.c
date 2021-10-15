#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <math.h>

int a[32]; // the global array

unsigned long crash_array(int n) {
    int i;
    // a[32] = 1; NOTE: this doesn't always crash
    unsigned long sum = 0;
    if (n > sizeof(a)/sizeof(int))
        n = sizeof(a)/sizeof(int);

    printf("%d\n", n);
    
    for (i = 0; i < n; i++) {
        sum = sum + a[i];
    }

    return sum;
}

unsigned long sum(int n) {
    int i;
    unsigned long sum = 0;
    for (i = 0; i < n; i++) {
        sum = sum + i;
    }

    return sum;
}

int main(void) {
    
    printf("Hello world\n"); 

    unsigned long s;
    s = sum(100);
    printf("Sum:%ld\n", s);

    // int i;
    // for (i = 0; i < 32; i++)
    // {
    //     a[i] = i;
    // }
    
    s = crash_array(100000);
    printf("crash array sum:%ld\n", s); 
    return 0; 
}
