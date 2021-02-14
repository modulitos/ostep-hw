// 1. Write a short C program (called `check-xor.c`) that computes an XOR-based
// checksum over an input file, and prints the checksumas output. Use a 8-bit
// unsigned char to store the (one byte) checksum. Make some test files to see
// if it works as expected.

#include "checksums.h"
#include <stdlib.h> // exit

int main(int argc, char * argv[]) {
    char * pathname = "";

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filepath]\n", argv[0]);
        return(EXIT_FAILURE);
    }

    pathname = argv[1];
    double t = Time_GetSeconds();
    char xor = check_xor(pathname);

    printf("Time (seconds): %f\n", Time_GetSeconds() - t);
    printf("XOR-based checksum: %d\n", xor);

    return(EXIT_SUCCESS);
}