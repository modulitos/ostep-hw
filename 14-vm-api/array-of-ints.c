#include <stdio.h>
#include <stdlib.h>

int main() {
    // int* ints = malloc(sizeof(int) * 100);
    int *data = (int *) malloc(sizeof(int) * 100);

    // if we don't initialize the values we're accessing, then valgrind will complain.
    data[99] = 42;

    printf("int[99]: %d\n", data[99]);

    // writing/reading to a out of bounds array item:
    // data[100] = 43;
    // printf("int[100]: %d\n", data[100]);

    // test passing a funny value to free:
    free(&data[50]);
    // free((void *) &data[50]);

    free(data);

    // accessing array item after free:
    // printf("int[99] after free: %d\n", data[99]);


    return 0;
}
