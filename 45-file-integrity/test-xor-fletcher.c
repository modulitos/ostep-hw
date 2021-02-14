// 1. Write a short C program (called `check-xor.c`) that computes an XOR-based
// checksum over an input file, and prints the checksumas output. Use a 8-bit
// unsigned char to store the (one byte) checksum. Make some test files to see
// if it works as expected.

#include "checksums.h"
#include <stdlib.h> // exit
#include <assert.h>

int main(int argc, char * argv[]) {
    char * pathname = "";

    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return(EXIT_FAILURE);
    }

    char xor = 0;

    // a: 0b1100001
    // b: 0b1100010
    // c: 0b1100011
    // d: 0b1100100
    pathname = "ab.txt";
    xor = check_xor(pathname);
    assert(xor == 3);


    pathname = "abd.txt";
    xor = check_xor(pathname);
    // printf("xor: %d\n", xor);
    assert(xor == 103);


    pathname = "abcd.txt";
    xor = check_xor(pathname);
    assert(xor == 4);

    return(EXIT_SUCCESS);
}
