#ifndef HOMEWORK_CHECKSUMS_H
#define HOMEWORK_CHECKSUMS_H

#include <unistd.h>
#include "common.h"
#include <fcntl.h> // open

char check_xor(char *pathname) {
    char buf[1] = "";
    size_t count = 0;
    char xor = 0;
    int fd;

    if ((fd = open(pathname, O_RDONLY)) == -1) {
        handle_error("open");
    }

    while ((count = read(fd, buf, 1)) != -1 && count != 0) {
        if (buf[0] != '\n') {
            // printf("xor: %i, buf[0]: %i, buf[0] - '0': %i\n", xor, buf[0], buf[0] - '0');

            // xor ^= (buf[0] - '0'); // converts the buf[0] value to a char?
            xor ^= buf[0];
        }
    }
    close(fd);
    return xor;
}

#endif