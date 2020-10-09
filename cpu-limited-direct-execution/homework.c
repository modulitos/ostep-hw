#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
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

//void context_switch_timers() {
//    const pid_t pid = getpid();
//    sched_setaffinity();
//}
//
int main() {
    sys_call_timers();
    return 0;
}

