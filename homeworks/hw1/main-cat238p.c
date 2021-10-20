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

void cat(int fd) {
  int n;

  char buf[512];
  for (;;) {
    n = read(fd, buf, sizeof buf);
    if (n == 0)
      break;
    if (n < 0) {
      fprintf(stderr, "read error\n");
      exit(1);
    }
    if (write(1, buf, n) != n) {
      fprintf(stderr, "write error\n");
      exit(1);      
    }
  }
}

int main(int argc, char *argv[])
{
  // printf("\n%d %s\n", argc, argv[1]);
  if(argc <= 1) {
    cat(0);
  } else if (argc == 2) {
    int fd = open(argv[1], 0);
    if (fd < 0) {
      fprintf(stderr, "canot open filename: %s\n", argv[1]);
      exit(1);
    }
    cat(fd);
    close(fd);
  } else {
    fprintf(stderr, "provide either no argument to read from stdin else provide filename to read");
    exit(1);
  }
  return 0;
}
