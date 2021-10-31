#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
static char b[8192];
int foo(char *p) {
    write(1, "hello\n", 6);
    foo(p);
    return 0;
}
int main() {
    char a[8192];
    foo(a);
    write(1, b, 10);
    exit(0);
}