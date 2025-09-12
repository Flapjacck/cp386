#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../../common.h"

volatile int counter = 0;
int loops;

void *worker(void *arg) {
    (void)arg; // Suppress unused parameter warning
    for (int i = 0; i < loops; i++) {
        counter++;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_loops>\n", argv[0]);
        exit(1);
    }
    
    loops = atoi(argv[1]);
    pthread_t p1, p2;
    printf("Initial value : %d\n", counter);
    
    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);
    
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    
    printf("Final value : %d\n", counter);
    printf("Expected value: %d\n", loops * 2);
    
    return 0;
}
