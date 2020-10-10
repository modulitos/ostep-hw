#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sched.h>

// using a macro:
// taken from here: https://www.reddit.com/r/C_Programming/comments/44siua/how_to_perform_0_byte_read/
#define TIME_10X(exp) ({  \
                    struct timeval ts, te;  \
                    gettimeofday(&ts, NULL);\
                      \
                    int i; \
                    for (i = 0; i < 10; i++) { \
                        exp; \
                    } \
                    gettimeofday(&te, NULL); \
                    unsigned long long uss = ts.tv_sec * 1000000 + ts.tv_usec, \
                      use = te.tv_sec * 1000000 + te.tv_usec; \
                    printf("%lluu\t- %s\n", use - uss, #exp); \
                    use - uss; \
                  })

void durationOf0ByteRead_10x() {
    struct timeval ts, te;
    int rc = gettimeofday(&ts, NULL);
    assert(rc == 0);
    int i;
    for (i = 0; i < 10; i++) {
        // This is our 0 byte read:
        read(0, NULL, 0);
    }
    int rc_2 = gettimeofday(&te, NULL);
    assert(rc_2 == 0);
    unsigned long long uss = ts.tv_sec * 1000000 + ts.tv_usec;
    unsigned long long use = te.tv_sec * 1000000 + te.tv_usec;
    // `llu` represents an "unsigned long long" in format print.
    printf("%lluus\t - %s\n", use - uss, "null byte read");
}

void sys_call_timers()
{
    // Lets run each timer 8 times to see what the average is:
    int i;
    for (i = 0; i < 8; i++) {
        durationOf0ByteRead_10x();
    }
    // alternative solution, using the macro:
    for (i = 0; i < 8; i++) {
        TIME_10X(read(0, NULL, 0));
    }
}


// given the following results:

// ❯ ./homework.o
// 14us     - null byte read
// 5us      - null byte read
// 4us      - null byte read
// 4us      - null byte read
// 5us      - null byte read
// 4us      - null byte read
// 4us      - null byte read
// 4us      - null byte read
// 4u      - read(0, NULL, 0)
// 5u      - read(0, NULL, 0)
// 4u      - read(0, NULL, 0)
// 4u      - read(0, NULL, 0)
// 4u      - read(0, NULL, 0)
// 4u      - read(0, NULL, 0)
// 5u      - read(0, NULL, 0)
// 4u      - read(0, NULL, 0)

// we can assume that each sys call takes ~.4-.5 us.

// unfortunately thread/CPU affinity api's are not available on macos, making the second part of this assignment impossible:
// https://developer.apple.com/library/archive/releasenotes/Performance/RN-AffinityAPI/index.html
// https://linux.die.net/man/3/cpu_set

// // potential workaround,
// // http://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html
//typedef struct cpu_set {
//    uint32_t    count;
//} cpu_set_t;
//
//static inline void
//CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }
//
//static inline void
//CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }
//

// This only works on Linux, where the sched_setaffinity api is available.

//void context_switch_timers() {
//    int nloops = 1000000;
//    struct timeval start, end;
//
//    cpu_set_t set;
//    CPU_ZERO(&set);
//    CPU_SET(0, &set);
//
//    int first_pipefd[2], second_pipefd[2];
//    if (pipe(first_pipefd) == -1) {
//        perror("pipe");
//        exit(EXIT_FAILURE);
//    }
//    if (pipe(second_pipefd) == -1) {
//        perror("pipe");
//        exit(EXIT_FAILURE);
//    }
//    pid_t cpid = fork();
//
//    if (cpid == -1) {
//        perror("fork");
//        exit(EXIT_FAILURE);
//    } else if (cpid == 0) {    // child
//        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
//            exit(EXIT_FAILURE);
//        }
//
//        for (size_t i = 0; i < nloops; i++) {
//            read(first_pipefd[0], NULL, 0);
//            write(second_pipefd[1], NULL, 0);
//        }
//    } else {           // parent
//        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
//            exit(EXIT_FAILURE);
//        }
//
//        gettimeofday(&start, NULL);
//        for (size_t i = 0; i < nloops; i++) {
//            write(first_pipefd[1], NULL, 0);
//            read(second_pipefd[0], NULL, 0);
//        }
//        gettimeofday(&end, NULL);
//        printf("context switch: %f microseconds\n", (float) (end.tv_sec * 1000000 + end.tv_usec - start.tv_sec * 1000000 - start.tv_usec) / nloops);
//    }
//}

int main() {
    sys_call_timers();
    // context_switch_timers();
    return 0;
}

