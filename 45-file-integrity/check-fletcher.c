// Now write a short C program (called `check-fletcher.c`) that computes the
// Fletcher checksum over an input file. Once again, test your program to see if
// it works.

#include <stdlib.h> // exit
#include "common.h"
#include "check-fletcher.h"

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
