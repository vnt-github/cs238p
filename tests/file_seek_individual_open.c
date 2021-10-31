#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int BUFFER_SIZE = 64;
    char buffer[BUFFER_SIZE];
   

    pid_t pid = fork();

    // parent
    if (pid > 0) {
        int fd = open(argv[1], O_RDONLY);
        //Read one byte
        ssize_t bytes_read = read(fd, buffer, 1);
        buffer[bytes_read] = '\0';
        printf("Move offset by one byte before fork(): expected 'a', got: '%s'\n", buffer);
        //Read 2 bytes
        bytes_read = read(fd, buffer, 2);
        buffer[bytes_read] = '\0';

        printf("Parent: move offset by 2 bytes after fork(): expected 'bc', got: '%s'\n", buffer);
        close(fd);

        sleep(2);
    }

    // child
    else {
        // Wait for 1s.
        sleep(1);
        int fd = open(argv[1], O_RDONLY);

        //Read one byte
        ssize_t bytes_read = read(fd, buffer, 1);
        buffer[bytes_read] = '\0';
        printf("Move offset by one byte before fork(): expected 'a', got: '%s'\n", buffer);

        // Read 1 byte
        bytes_read = read(fd, buffer, 1);
        buffer[bytes_read] = '\0';
        printf("Child: move offset by one byte after fork(): expected 'b', got: '%s'\n", buffer);
        close(fd);
    }

    return 0;
}
