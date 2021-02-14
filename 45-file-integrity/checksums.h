#ifndef HOMEWORK_CHECKSUMS_H
#define HOMEWORK_CHECKSUMS_H

#include "common.h"
#include <fcntl.h> // open
#include <unistd.h>

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
            // printf("xor: %i, buf[0]: %i, buf[0] - '0': %i\n", xor, buf[0],
            // buf[0] - '0');

            // xor ^= (buf[0] - '0'); // converts the buf[0] value to a char?
            xor ^= buf[0];
        }
    }
    close(fd);
    return xor;
}

typedef struct Fletcher_t {
    int fletcher_a;
    int fletcher_b;
    // TODO: is there a way to make these only 1 byte long???
    // char fletcher_a;
    // char fletcher_b;
} Fletcher_t;

void init_fletcher(struct Fletcher_t *f) {
    f->fletcher_a = 0;
    f->fletcher_b = 0;
}

void fletcher_process_byte(struct Fletcher_t *f, char b) {
    f->fletcher_a = ((b + f->fletcher_a) % 255);
    f->fletcher_b = ((f->fletcher_b + f->fletcher_a) % 255);
}

Fletcher_t check_fletcher(char *pathname) {
    char buf[1] = "";
    size_t count = 0;
    Fletcher_t res;
    init_fletcher(&res);
    int fd;

    if ((fd = open(pathname, O_RDONLY)) == -1) {
        handle_error("open");
    }

    while ((count = read(fd, buf, 1)) != -1 && count != 0) {
        if (buf[0] != '\n') {

            fletcher_process_byte(&res, buf[0]);
            // printf("fletcher_a: %i, fletcher_b: %i, buf[0]: %i\n", res.fletcher_a, res.fletcher_b, buf[0]);
        }
    }
    close(fd);

    return res;
}

#endif