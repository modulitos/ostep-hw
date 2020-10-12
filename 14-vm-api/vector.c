#include "vector.h"
#include <assert.h>

int main() {
    // initialize our vector with a capacity of 2:
    struct vector vec = {
        // .data = (int*) malloc(sizeof(int) * 2),
        .data = (int*) calloc(2, sizeof(int)),
        .capacity = 2,
        .size = 0
    };
    struct vector* vp = &vec;
    vector_append(vp, 42);
    vector_append(vp, 99);
    vector_append(vp, 34);

    assert(vector_get(vp, 0) == 42);
    assert(vector_get(vp, 1) == 99);
    assert(vector_get(vp, 2) == 34);

    assert(vector_pop(vp) == 34);
    assert(vector_pop(vp) == 99);
    assert(vector_pop(vp) == 42);

    vector_free(vp);
    return 0;
}