#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * binary_semaphore.c
 *
 * This program demonstrates the use of a binary semaphore as a mutex
 * to protect a critical section. It shows how semaphores can be used
 * for basic mutual exclusion, similar to pthread_mutex_t.
 */

#define NUM_THREADS 5
#define ITERATIONS 100000

// Shared global variable
int counter = 0;

// Binary semaphore (used as mutex)
sem_t mutex;

// Thread function - increments counter in a critical section
void* increment_counter(void* arg) {
    int id = *((int*)arg);
    int local_counter = 0;
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Enter critical section
        sem_wait(&mutex);
        
        // Critical section - increment shared counter
        counter++;
        local_counter++;
        
        // Exit critical section
        sem_post(&mutex);
    }
    
    printf("Thread %d: Completed %d increments, counter should be: %d\n",
           id, local_counter, local_counter * NUM_THREADS);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("Binary Semaphore Demonstration (Mutex)\n");
    printf("--------------------------------------\n");
    
    // Initialize the semaphore with value 1 (binary semaphore)
    if (sem_init(&mutex, 0, 1) != 0) {
        perror("sem_init");
        return 1;
    }
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Check if the counter has the expected value
    int expected = NUM_THREADS * ITERATIONS;
    printf("\nFinal counter value: %d\n", counter);
    printf("Expected value: %d\n", expected);
    
    if (counter == expected) {
        printf("SUCCESS: Counter matches expected value!\n");
    } else {
        printf("ERROR: Counter does not match expected value!\n");
    }
    
    // Destroy the semaphore
    sem_destroy(&mutex);
    
    return 0;
}