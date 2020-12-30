#include "zemaphore.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __ns_mutex_t {
    int room1_counts;
    int room2_counts;
    Zem_t turnstile_1;
    Zem_t turnstile_2;
    Zem_t mutex;
} ns_mutex_t;

void ns_mutex_init(ns_mutex_t *m) {
    Zem_Init(&m->turnstile_1, 1);
    Zem_Init(&m->turnstile_2, 1);
    Zem_Init(&m->mutex, 1);
    m->room1_counts = 0;
    m->room2_counts = 0;
}

void ns_mutex_acquire(ns_mutex_t *m) {
    // Room 1
    Zem_wait(&m->mutex);
    m->room1_counts++;
    Zem_post(&m->mutex);

    // Room 2
    Zem_wait(&m->turnstile_1);
    m->room2_counts++;
    Zem_wait(&m->mutex);
    m->room1_counts--;

    if (m->room1_counts == 0) {
        Zem_post(&m->mutex);
        Zem_post(&m->turnstile_2);
    } else {
        Zem_post(&m->mutex);
        Zem_post(&m->turnstile_1);
    }

    // Room 3
    Zem_wait(&m->turnstile_2);
    m->room2_counts--;

    // crticial worker section

    if (m->room2_counts == 0) {
        Zem_post(&m->turnstile_1);
    } else {
        Zem_post(&m->turnstile_2);
    }
}

void ns_mutex_release(ns_mutex_t *m) {}

// TESTING

int loops;
int value = 0;
ns_mutex_t mutex;

void *worker(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        ns_mutex_acquire(&mutex);
        value++;
        printf("worker value: %d\n", value);
        ns_mutex_release(&mutex);
    }
    sleep(1);
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 3);
    int num_workers = atoi(argv[1]);
    loops = atoi(argv[2]);

    printf("args parsed!\n");
    pthread_t workers[num_workers];

    ns_mutex_init(&mutex);

    printf("parent: begin\n");
    int i;
    for (i = 0; i < num_workers; i++) {
        Pthread_create(&workers[i], NULL, worker, NULL);
    }

    for (i = 0; i < num_workers; i++) {
        Pthread_join(workers[i], NULL);
    }
    printf("parent: end\n");

    printf("final value: %d\n", value);
    assert(value == num_workers * loops);
    return 0;
}
