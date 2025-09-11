#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../common.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv; // Suppress unused parameter warnings
    int *p = malloc(sizeof(int));   // a1
    assert(p != NULL);
    printf("(%d) address of p: %p\n", getpid(), (void*) p); // a2
    *p = 0;                         // a3
    while (1) {
        Spin(1);
        *p = *p + 1;
        printf("(%d) value of p: %d and addr pointed to by p: %p\n",
               getpid(), *p, (void*) p);    // a4
    }
    return 0;
}
