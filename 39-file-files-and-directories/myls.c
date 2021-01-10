#include <stdbool.h>
#include <stdio.h>
#include <dirent.h> // opendir  https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h
#include <sys/stat.h> // stat https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/sys/stat.h#Member_types
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h> // getopt: https://www.gnu.org/software/libc/manual/html_node/Using-Getopt.html
#include <string.h>    // strlen, strncpy, strncmp, strncat
#include "./common.h"

// Write a program that lists files in the given directory.When called without
// any arguments, the program should just print the file names. When invoked
// with the-lflag, the program should print out information about each file,
// such as the owner, group, per-missions, and other information obtained from
// the `stat()` systemcall. The program should take one additional argument,
// which is the directory to read, e.g.`myls -l directory`. If no directory is
// given, the program should just use the current working directory. Useful
// interfaces: stat(), opendir(), readdir(), getcwd()

int parse_args(int argc, char *argv[], bool *list, char **pathname) {
    int args_to_parse = argc - 1;
    int optres = getopt(argc, argv, "l");
    if (optres != -1) {
        args_to_parse--;
        if (optres == 'l') {
            *list = true;
        } else {
            printf("got ?\n");
            perror("unexpected arg");
            exit(EXIT_FAILURE);
        }
    }
    if (args_to_parse == 1) {
        // a custom dirname was passed in, which will always be the last arg:
        *pathname = argv[argc - 1];
    } else if (args_to_parse > 1) {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *pathname = ".";
    bool list = false;

    int parse_res = parse_args(argc, argv, &list, &pathname);
    if (parse_res == -1) {
        perror("options parsing");
        exit(EXIT_FAILURE);
    }

    printf("pathname: %s\n", pathname);
    printf("list: %d, path: %s\n", list, pathname);

    struct stat sb;

    if (stat(pathname, &sb) != 0) {
        perror("stat"); // just a string to indicate the process that failed
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(sb.st_mode)) {
        // A structure representing a directory stream:
        DIR* dp;
        if ((dp = opendir(pathname)) == NULL) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }
        struct dirent *dir_entry;
        while ((dir_entry = readdir(dp)) != NULL) {

            // iterate through each directory entry, printing it's info and any
            // details

            printf("dir_entry->name: %s\n", dir_entry->d_name);
            if (list) {
                char filepath[FILENAME_MAX] = "";
                // https://en.wikibooks.org/wiki/C_Programming/string.h
                strncpy(filepath, pathname, strlen(pathname));
                strncat(filepath, "/", 1);
                strncat(filepath, dir_entry->d_name, strlen(dir_entry->d_name));
                printf("file: %s!\n", filepath);
                struct stat file_details;
                if (stat(filepath, &file_details) != 0) {
                    perror("stat on file");
                    exit(EXIT_FAILURE);
                }
                print_file_details(file_details);
                printf("\n");
            }
        }
        closedir(dp);
    } else {
        printf("file: %s\n", pathname);
        if (list) {
            printf("file details:\n");
            print_file_details(sb);
        }
    }

    exit(EXIT_SUCCESS);
}