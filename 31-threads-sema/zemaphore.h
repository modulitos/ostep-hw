#include "common_threads.h"

#ifndef HOMEWORK_ZEMAPHORE_H
#define HOMEWORK_ZEMAPHORE_H

#endif // HOMEWORK_ZEMAPHORE_H

typedef struct Zem_t {
  int value;
  pthread_cond_t cond;
  pthread_mutex_t lock;
} Zem_t;

// only one thread can call this:
void Zem_Init(Zem_t *s, int value) {
  Mutex_init(&s->lock);
  Cond_init(&s->cond);
  s->value = value;
}

void Zem_wait(Zem_t *s) {
  Mutex_lock(&s->lock);
  while (s->value <= 0) {
    Cond_wait(&s->cond, &s->lock);
  }
  s->value--;
  Mutex_unlock(&s->lock);
}

// NOTE: we don’t maintain the invariant that the value of the semaphore, when
// negative, reflects the number of waiting threads; indeed, the value will
// never be lower than zero. This behavior is easier to implement and matches
// the current Linux implementation.

void Zem_post(Zem_t *s) {
  Mutex_lock(&s->lock);
  s->value++;
  Cond_signal(&s->cond);
  Mutex_unlock(&s->lock);
}