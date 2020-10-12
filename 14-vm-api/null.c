#include <stdio.h>
#include <stdlib.h>


int main() {
    // int* p = calloc(1, sizeof(int));
    int* p = malloc(sizeof(int));

    // this isn't needed, since it's 0 by default. But it's best practice, and
    // omitting it will raise an error in valgrind.
    *p = 0;

    printf("value of int pointer: %d\n", *p);

    // creating a null pointer and then dereferencing it:
    // p = NULL;
    // printf("address of nulled int pointer: %p\n", p);
    // printf("value of int pointer: %d\n", *p);

    // try commenting this out to see what gdb and/or valgrind does:
    free(p);
    return 0;
}