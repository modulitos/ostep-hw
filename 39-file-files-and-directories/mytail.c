// Write a program that prints out the last few lines of a file. The program
// should be efficient, in that it seeks to near the end of the file, reads in a
// block of data, and then goes backwards until it finds the requested number of
// lines; at this point, it should print out those lines from beginning to the
// end of the file. To invoke the program,one should type: `mytail -n file`,
// where n is the number of lines at the end of the file to print. Useful
// interfaces: `stat()`, `lseek()`, `open()`, `read()`, `close()`.

#include "./common.h" // print_file_details
#include <fcntl.h>    // open
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strlen
#include <sys/stat.h>
#include <unistd.h> // lseek, read, https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/unistd.h

// Not sure why we do this in a loop, but it seems like a best practice, and it
// can't hurt:
#define handle_error(msg)                                                      \
    do {                                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

int main(int argc, char *argv[]) {

    if (argc != 3 || strlen(argv[1]) > 2 || argv[1][0] != '-') {
        fprintf(stderr, "Usage: %s -<offset> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int lines_to_print = atoi(argv[1]);

    // since this is passed in as a flage (eg: `-5`), it will be parsed as a
    // negative number. So let's make it positive:

    lines_to_print *= -1;
    // incrementing this since read() inserts a newline at the end of the file.
    lines_to_print++;

    // open the file, and read its contents:
    char *pathname = argv[2];
    struct stat sb;
    if (stat(pathname, &sb) != 0) {
        handle_error("stat");
    }
    // print_file_details(sb);

    int fd;
    if ((fd = open(pathname, O_RDONLY)) == -1) {
        handle_error("open");
    }
    // printf("fd: %d", fd);

    // lseek is also part of unistd.h:
    // https://man7.org/linux/man-pages/man2/lseek.2.html
    if (lseek(fd, -1, SEEK_END) == -1) {
        handle_error("lseek");
    }

    // This is where our results will be stored and printed:
    char buf[sb.st_size];

    int offset;
    while (lines_to_print > 0) {
        if (read(fd, buf, 1) == -1) {
            handle_error("read");
        }
        if (buf[0] == '\n') {
            lines_to_print--;
        }
        offset = lseek(fd, -2, SEEK_CUR);
        if (offset == -1) {
            break;
        }
    }

    if (offset > 0 || lines_to_print == 0) {

        // If the entire file was not parsed, then move the offset forward 2
        // bytes for account for the last `lseek`:

        if (lseek(fd, 2, SEEK_CUR) == -1)
            handle_error("lseek");
    } else {
        // handle the case where the entire file was parsed:
        if (lseek(fd, 0, SEEK_SET) == -1)
            handle_error("lseek");
    }

    memset(buf, 0, sb.st_size);
    if (read(fd, buf, sb.st_size) == -1) {
        handle_error("read");
    }

    printf("results:\n%s\n", buf);
    close(fd);

    exit(EXIT_SUCCESS);
}