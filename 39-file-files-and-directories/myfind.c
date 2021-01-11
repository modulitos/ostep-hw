// Recursive Search: Write a program that prints out the names of each file and
// directory in the file system tree, starting at a given point in the tree. For
// example, when run without arguments, the program should start with the
// current working directory and print its contents, as well as the contents of
// any sub-directories, etc., until the entire tree, root at the CWD, is
// printed. If given a single argument (of a directory name), use that as the
// root of the tree instead. Refine your recursive search with more fun options,
// similar to the powerful `find` command line tool.

#include <dirent.h>
#include <errno.h> // errno, EACCES
#include <regex.h> // https://www.gnu.org/software/libc/manual/html_node/Regular-Expressions.html
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Not sure why we do this in a loop, but it seems like a best practice, and it
// can't hurt:
#define handle_error(msg)                                                      \
    do {                                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

/// Prints all contents inside the directory, recursively.
void *find_dir(char *dirpath, int current_depth, int max_depth, regex_t *preg) {

    if (current_depth > max_depth) {
        exit(EXIT_SUCCESS);
    }
    errno = 0;

    DIR *dp; // our directory entries stream
    if ((dp = opendir(dirpath)) == NULL) {
        if (errno == EACCES) {
            fprintf(stderr, "myfind: ‘%s‘: Permission denied\n", dirpath);
            return 0;
        } else {
            handle_error("opendir");
        }
    }
    struct dirent *dir_entry;
    while ((dir_entry = readdir(dp)) != NULL) {
        // set the child path the be prefixed by the dir's path, then a '/',
        // then the dir_entry's name

        char childpath[FILENAME_MAX] = "";
        strncpy(childpath, dirpath, strlen(dirpath));
        if (childpath[strlen(childpath) - 1] != '/') {
            // add the trailing `/`, if it's not already present:
            strncat(childpath, "/", 1);
        }
        strncat(childpath, dir_entry->d_name, strlen(dir_entry->d_name));

        if (strncmp(dir_entry->d_name, "..", 2) != 0 &&
            strncmp(dir_entry->d_name, ".", 1) != 0) {
            if (preg == NULL ||
                regexec(preg, childpath, 0, NULL, 0) != REG_NOMATCH) {
                // print the dirent, assuming the regex matches or is disabled:
                printf("%s\n", childpath);
            }

            if (dir_entry->d_type == DT_DIR) {
                find_dir(childpath, current_depth + 1, max_depth, preg);
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {

    char *pathname = ".";
    bool enable_pattern = false;
    int max_depth = INT32_MAX;

    regex_t preg;
    char *pattern = "";

    // getopt pattern re-used from here:
    // https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html

    int opt;
    while ((opt = getopt(argc, argv, "d:p:")) != -1) {
        switch (opt) {
        case 'd':
            // max depth is defined:
            if (optarg < 0) {
                fprintf(stderr, "Max depth must be positive.\n");
                exit(EXIT_FAILURE);
            }
            max_depth = atoi(optarg);
            break;
        case 'p':
            // regex matcher is defined:
            enable_pattern = true;
            pattern = optarg;
            break;
        default:
            break;
        }
    }

    if (argc > 3 && optind == 1) {
        fprintf(stderr, "Usage: %s -d [max depth] -n [pattern] [filepath]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    if (optind == argc - 1) {

        // if there was a custom pathname passed in (which is not associated
        // with any of our getopt cli flags)

        pathname = argv[optind];
    }

    if (enable_pattern && regcomp(&preg, pattern, REG_NOSUB) != 0) {
        handle_error("regcomp");
    }

    // Print out the root path
    if (!enable_pattern ||
        regexec(&preg, pathname, 0, NULL, 0) != REG_NOMATCH) {
        printf("%s\n", pathname);
    }

    struct stat sb;
    if (stat(pathname, &sb) == -1) {
        printf("the root path was not found\n");
        handle_error("stat");
    }

    if (S_ISDIR(sb.st_mode)) {
        if (enable_pattern) {
            find_dir(pathname, 1, max_depth, &preg);
        } else {
            find_dir(pathname, 1, max_depth, NULL);
        }
    }

    exit(EXIT_SUCCESS);
}