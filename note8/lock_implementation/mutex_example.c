#include <stdio.h>
#include <pthread.h>

// Simple mutex example demonstrating race condition resolution

// Shared global variable
int counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Unsafe increment function - susceptible to race conditions
void* unsafe_increment(void* arg) {
    int i;
    int iterations = *((int*)arg);
    
    for (i = 0; i < iterations; i++) {
        // Critical section: read-modify-write operation
        counter = counter + 1;
    }
    
    return NULL;
}

// Safe increment function - protected by mutex
void* safe_increment(void* arg) {
    int i;
    int iterations = *((int*)arg);
    
    for (i = 0; i < iterations; i++) {
        // Enter critical section
        pthread_mutex_lock(&mutex);
        
        // Safely increment counter
        counter = counter + 1;
        
        // Exit critical section
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int iterations = 1000000; // Each thread will increment 1 million times
    
    // Part 1: Demonstrate race condition with unsafe increment
    counter = 0;
    printf("Starting unsafe increment test...\n");
    
    // Create two threads that both call unsafe_increment
    pthread_create(&thread1, NULL, unsafe_increment, &iterations);
    pthread_create(&thread2, NULL, unsafe_increment, &iterations);
    
    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // The expected result is 2 million, but due to race conditions,
    // the actual result will almost always be less
    printf("Unsafe increment: Expected value: %d, Actual value: %d\n", 
           2 * iterations, counter);
    
    // Part 2: Demonstrate proper synchronization with mutex
    counter = 0;
    printf("\nStarting safe increment test...\n");
    
    // Create two threads that both call safe_increment
    pthread_create(&thread1, NULL, safe_increment, &iterations);
    pthread_create(&thread2, NULL, safe_increment, &iterations);
    
    // Wait for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // With proper synchronization, the result should be exactly 2 million
    printf("Safe increment: Expected value: %d, Actual value: %d\n", 
           2 * iterations, counter);
    
    pthread_mutex_destroy(&mutex);
    return 0;
}