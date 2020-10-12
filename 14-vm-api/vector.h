#include <stdio.h>
#include <stdlib.h>

struct vector {
    int* data;
    int size;
    int capacity;
};

void vector_append(struct vector* v, int val) {
    // realloc when vector is too full:
    if (v->size >= v->capacity) {
        v->capacity = v->capacity * 2;
        v->data = realloc(v->data, v->capacity * sizeof(int));
    }
    (v->data)[v->size] = val;
    v->size = v->size + 1;
}

int vector_pop(struct vector* v) {
    if (v->size == 0) {
        printf("vector error: cannot pop when size is zero!");
        exit(1);
    }
    (v->size)--;
    return (v->data)[v->size];
}

int vector_get(struct vector* v, int i) {
    if (i >= v->size) {
        printf("vector error: index %d is greater and vec size: %d\n", i, v->size);
        exit(1);
    }
    if (i >= v->capacity) {
        printf("vector error: index %d is greater and vec capacity: %d\n", i, v->size);
        exit(1);
    }
    return v->data[i];
}

void vector_free(struct vector* v) {
    free(v->data);
    v->size = 0;
    v -> capacity = 0;
}