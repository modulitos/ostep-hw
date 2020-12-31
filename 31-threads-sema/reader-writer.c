#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "zemaphore.h"
#include "lightswitch.h"

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
    // int readers;
    // Zem_t mutex;
    Zem_t room_empty;

    // alternative impl using Lightswitch:
    Lightswitch_t lightswitch;
} rwlock_t;


void rwlock_init(rwlock_t *rw) {
    // Zem_Init(&rw->mutex, 1);
    // rw->readers = 0;
    Zem_Init(&rw->room_empty, 1);

    Lightswitch_Init(&rw->lightswitch);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    // Zem_wait(&rw->mutex);
    // rw->readers++;
    // if (rw->readers == 1) {
    //     Zem_wait(&rw->room_empty);
    // }
    // Zem_post(&rw->mutex);

    Lightswitch_Lock(&rw->lightswitch, &rw->room_empty);
}

void rwlock_release_readlock(rwlock_t *rw) {
    // Zem_wait(&rw->mutex);
    // rw->readers--;
    // if (rw->readers == 0) {
    //     Zem_post(&rw->room_empty);
    // }
    // Zem_post(&rw->mutex);

    Lightswitch_Unlock(&rw->lightswitch, &rw->room_empty);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    Zem_wait(&rw->room_empty);
}

void rwlock_release_writelock(rwlock_t *rw) {
    Zem_post(&rw->room_empty);
}

//
// Don't change the code below (just use it!)
// 

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
	rwlock_acquire_readlock(&lock);
	printf("read %d\n", value);
	rwlock_release_readlock(&lock);
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        sleep(1);
	rwlock_acquire_writelock(&lock);
	value++;
	printf("write %d\n", value);
	rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
	Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
	Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}

