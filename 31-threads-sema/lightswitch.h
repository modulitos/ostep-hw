#include "zemaphore.h"

#ifndef HOMEWORK_LIGHTSWITCH_H
#define HOMEWORK_LIGHTSWITCH_H

#endif // HOMEWORK_LIGHTSWITCH_H

typedef struct Lightswitch_t {
    int counter;
    Zem_t mutex;
} Lightswitch_t;


// It would also be possible to store a reference to `roomEmpty` as an attribute
// of the Lightswitch, rather than pass it as a parameter to `lock` and
// `unlock`. This alternative would be less error-prone, but I think it improves
// readability if each invocation of `lock` and `unlock` specifies the semaphore
// it operates on.

void Lightswitch_Init(struct Lightswitch_t *l) {
    l->counter = 0;
    Zem_Init(&l->mutex, 1);
}

void Lightswitch_Lock(struct Lightswitch_t *l, struct Zem_t *s) {
    Zem_wait(&l->mutex);
    if (l->counter == 0) {
        Zem_wait(s);
    }
    l->counter++;
    Zem_post(&l->mutex);
}

void Lightswitch_Unlock(struct Lightswitch_t *l, struct Zem_t *s) {
    Zem_wait(&l->mutex);
    l->counter--;
    if (l->counter == 0) {
        Zem_post(s);
    }
    Zem_post(&l->mutex);
}
