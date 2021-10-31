#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

char buf[512];

void tee(int fd) {
  int n;

  char buf[512];
  for (;;) {
    n = read(0, buf, sizeof buf);
    if (n == 0)
      break;
    if (n < 0) {
      fprintf(stderr, "read error\n");
      exit(1);
    }
    if (write(1, buf, n) != n) {
      fprintf(stderr, "write error to stdout\n");
      exit(1);      
    }
    if (write(fd, buf, n) != n) {
      fprintf(stderr, "write error to file");
      exit(1);
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc == 2) {
    int fd = open(argv[1], O_WRONLY|O_CREAT, 0644);
    if (fd < 0) {
      fprintf(stderr, "cannot open filename: %s\n", argv[1]);
      exit(1);
    }
    tee(fd);
    close(fd);
  } else {
    fprintf(stderr, "provide either no argument to read from stdin else provide filename to read");
    exit(1);
  }
  return 0;
}
