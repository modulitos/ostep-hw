#include <stdio.h>
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <sys/stat.h>
#include "./common.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("invalid number of args: %d", argc);
        exit(EXIT_FAILURE);
    }

    char *file = argv[1];

    // Info on the stat struct:
    // https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/sys/stat.h#Member_types
    struct stat sb;

    int res = stat(file, &sb);
    if (res != 0) {
        perror("stat"); // just a string to indicate the process that failed
        exit(EXIT_FAILURE);
    }

    print_file_details(sb);

    exit(EXIT_SUCCESS);
}