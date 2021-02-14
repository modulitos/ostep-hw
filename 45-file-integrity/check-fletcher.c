// Now write a short C program (called `check-fletcher.c`) that computes the
// Fletcher checksum over an input file. Once again, test your program to see if
// it works.

#include <stdlib.h> // exit
#include "checksums.h"

int main(int argc, char * argv[]) {
    char * pathname = "";

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filepath]\n", argv[0]);
        return(EXIT_FAILURE);
    }

    Fletcher_t fletcher;
    pathname = argv[1];
    double t = Time_GetSeconds();
    fletcher = check_fletcher(pathname);

    printf("Fletcher Time (seconds): %f\n", Time_GetSeconds() - t);
    printf("fletcher a: %d\n", fletcher.fletcher_a);
    printf("fletcher b: %d\n", fletcher.fletcher_b);

    return(EXIT_SUCCESS);
}
