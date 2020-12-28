#include "./zemaphore.h"
#include "common_threads.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// sem_t s;
Zem_t s;

void *child(void *arg) {
    sleep(3);
    printf("child\n");
    // use semaphore here
    Zem_post(&s);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    printf("parent: begin\n");
    // init semaphore here
    Zem_Init(&s, 0);
    Pthread_create(&p, NULL, child, NULL);
    // use semaphore here
    Zem_wait(&s);
    printf("parent: end\n");
    return 0;
}
