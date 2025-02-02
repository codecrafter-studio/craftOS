#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "common.h"

#define BUFSIZE 16

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: cat <filename>\n");
        return 0;
    }

    int fd = open(argv[1], 0);
    if (fd == -1) {
        printf("unable to open file\n");
        return 0;
    }

    char buffer[BUFSIZE];
    ssize_t bytesread;

    while ((bytesread = read(fd, buffer, BUFSIZE)) > 0) {
        if (bytesread > BUFSIZE) {
            printf("Data exceeds buffer size\n");
            return 0;
        }
        printf("%s", buffer);
    }

    if (!endswith(buffer, "\n")) {
        printf("\n");
    }

    close(fd);

    return 0;
}
