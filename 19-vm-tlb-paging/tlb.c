#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

void loopAllPagesAsArray(int num_pages, int page_size, int* arr) {
    int jump = (int) (page_size / sizeof(int)); // 1024 bytes
    int i;
    for (i = 0; i < num_pages * jump; i += jump) {
        arr[i] += 1;
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "usage: tlb.o <num_pages> <num_trials>\n");
        exit(EXIT_FAILURE);
    }

    int num_pages  = (int) strtol(argv[1], NULL, 10);
    long num_trials  = strtol(argv[2], NULL, 10);

    if (num_pages <= 0) {
        fprintf(stderr, "num_pages must be greater than 0, but was: %d\n", num_pages);
        exit(EXIT_FAILURE);
    }
    if (num_trials <= 0) {
        fprintf(stderr, "num_trials must be greater than 0, but was: %lu\n", num_trials);
        exit(EXIT_FAILURE);
    }


    int page_size = (int) sysconf(_SC_PAGESIZE);
    // this is the same as:
    // int page_size = getpagesize();

    // Verify that we have 4KB (4096 bytes) page sizes:
    printf("pagesize: %i\n", page_size);

    // Allocate our array to represent our pages. We'll need 4096 * num_pages
    // bytes of memory for the array.
    //
    // If you don’t initialize the array above before accessing it, the first
    // time you access it will be very expensive, due to initial access costs
    // such as demand zeroing.
    //
    // option 1:
    // int* arr = malloc((size_t) page_size * (size_t) num_pages);
    // memset(arr, 0, (size_t) page_size * (size_t) num_pages); // option 1 to initialize arr with size 0
    //
    // option 2:
    int* arr = calloc(((size_t) page_size * (size_t) num_pages) / sizeof(int), sizeof(int));

    struct timeval ts, te;
    // set the start time:
    int rc = gettimeofday(&ts, NULL);
    assert(rc == 0);
    unsigned long long nss = ts.tv_sec * 1000000000 + (ts.tv_usec * 1000);

    int i;
    for (i = 0; i < num_trials; i++) {
        loopAllPagesAsArray(num_pages, page_size, arr);
    }

    // set the end time:
    int rc_2 = gettimeofday(&te, NULL);
    assert(rc_2 == 0);
    unsigned long long nse = (te.tv_sec * 1000000000) + (te.tv_usec * 1000);
    // `llu` represents an "unsigned long long" in format print.
    unsigned long long duration = nse - nss;
    printf("%llu ns\t - time per page.\n", duration / (num_pages * num_trials));

    free(arr);
    return 0;
}

