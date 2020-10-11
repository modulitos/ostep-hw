#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <time.h>

// accepts 2 args:
// * memory-mb: a size, in MB, that this program will consume.
// * duration-sec: a duration, in seconds, for how long this program will run.
int main(int argc, char *argv[]) {
    printf("pid: %d\n", getpid());
    if (argc != 3) {
        fprintf(stderr, "usage: memory-user.o <memory-mb> <duration-sec>\n");
        exit(1);
    }
    // size in MB:
    long mem  = strtol(argv[1], NULL, 10) * 1024 * 1024;
    int end  = (int) strtol(argv[2], NULL, 10);
    // int mem  = atoi(argv[1]) * 1024 * 1024;
    printf("mem: %lu\n", mem);

    long len = (long) (mem / (sizeof(int)));
    int *arr = malloc(mem);
    // start is the processor time:
    // https://en.cppreference.com/w/c/chrono/clock_t
    clock_t start = clock();
    printf("clock start: %lu\n", start);
    int i;
    while (1) {
        double time_spent = ((double) clock() - start) / CLOCKS_PER_SEC;
        if (time_spent > end) {
            break;
        };

        // constantly stream through the array, touching each entry. Otherwise
        // the compiler will optimize away our allocation:
        for (i = 0; i < len; i++) {
            arr[i] += 1;
        }
    }
    free(arr);
    return 0;
}