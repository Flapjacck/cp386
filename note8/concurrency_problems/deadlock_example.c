#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * This example demonstrates how deadlocks can occur and how to prevent them
 * by using a consistent lock ordering strategy.
 */

// Two mutex locks
pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

// Global counters
int counter1 = 0;
int counter2 = 0;

// Thread that acquires locks in order A->B
void* thread_function_1(void* arg) {
    int iterations = 10000;
    
    printf("Thread 1 starting: will acquire locks in order A->B\n");
    
    for (int i = 0; i < iterations; i++) {
        // Acquire locks in order: mutex_A first, then mutex_B
        pthread_mutex_lock(&mutex_A);
        printf("Thread 1 acquired mutex A\n");
        
        // Simulate some work
        usleep(10);
        
        pthread_mutex_lock(&mutex_B);
        printf("Thread 1 acquired mutex B\n");
        
        // Critical section protected by both locks
        counter1++;
        counter2++;
        
        // Release locks in reverse order (best practice)
        pthread_mutex_unlock(&mutex_B);
        pthread_mutex_unlock(&mutex_A);
    }
    
    printf("Thread 1 completed\n");
    return NULL;
}

// Thread that acquires locks in order B->A (can cause deadlock)
void* thread_function_2_deadlock(void* arg) {
    int iterations = 10000;
    
    printf("Thread 2 starting: will acquire locks in order B->A (potential deadlock)\n");
    
    for (int i = 0; i < iterations; i++) {
        // Acquire locks in different order: mutex_B first, then mutex_A
        pthread_mutex_lock(&mutex_B);
        printf("Thread 2 acquired mutex B\n");
        
        // Simulate some work
        usleep(10);
        
        pthread_mutex_lock(&mutex_A);
        printf("Thread 2 acquired mutex A\n");
        
        // Critical section protected by both locks
        counter1++;
        counter2++;
        
        // Release locks in reverse order (best practice)
        pthread_mutex_unlock(&mutex_A);
        pthread_mutex_unlock(&mutex_B);
    }
    
    printf("Thread 2 completed\n");
    return NULL;
}

// Thread that acquires locks in the same order as thread_function_1 (no deadlock)
void* thread_function_2_safe(void* arg) {
    int iterations = 10000;
    
    printf("Thread 2 starting: will acquire locks in order A->B (consistent ordering)\n");
    
    for (int i = 0; i < iterations; i++) {
        // Acquire locks in the SAME order as thread_function_1: mutex_A first, then mutex_B
        pthread_mutex_lock(&mutex_A);
        printf("Thread 2 acquired mutex A\n");
        
        // Simulate some work
        usleep(10);
        
        pthread_mutex_lock(&mutex_B);
        printf("Thread 2 acquired mutex B\n");
        
        // Critical section protected by both locks
        counter1++;
        counter2++;
        
        // Release locks in reverse order (best practice)
        pthread_mutex_unlock(&mutex_B);
        pthread_mutex_unlock(&mutex_A);
    }
    
    printf("Thread 2 completed\n");
    return NULL;
}

// Demonstrate deadlock prevention using trylock with timeout and backoff
void* thread_function_2_trylock(void* arg) {
    int iterations = 10000;
    int success = 0;
    int failures = 0;
    
    printf("Thread 2 starting: will use trylock with backoff strategy\n");
    
    for (int i = 0; i < iterations; i++) {
        int acquired_all = 0;
        
        while (!acquired_all) {
            // Try to acquire mutex_B first
            if (pthread_mutex_trylock(&mutex_B) == 0) {
                // Got mutex_B, now try mutex_A
                if (pthread_mutex_trylock(&mutex_A) == 0) {
                    // Success - we have both locks
                    acquired_all = 1;
                    success++;
                    
                    // Critical section protected by both locks
                    counter1++;
                    counter2++;
                    
                    // Release locks
                    pthread_mutex_unlock(&mutex_A);
                    pthread_mutex_unlock(&mutex_B);
                } else {
                    // Failed to get mutex_A, release mutex_B and retry
                    pthread_mutex_unlock(&mutex_B);
                    failures++;
                    
                    // Random backoff to reduce contention
                    usleep(rand() % 1000);
                }
            } else {
                // Failed to get mutex_B, try again
                failures++;
                
                // Random backoff to reduce contention
                usleep(rand() % 1000);
            }
        }
    }
    
    printf("Thread 2 completed: %d successes, %d failures/retries\n", 
           success, failures);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int run_mode;
    
    printf("Deadlock demonstration:\n");
    printf("1. Safe mode - both threads use same lock order (A->B)\n");
    printf("2. Deadlock mode - threads use different lock orders (A->B and B->A)\n");
    printf("3. Trylock mode - use trylock with backoff to avoid deadlock\n");
    printf("Enter mode (1, 2, or 3): ");
    scanf("%d", &run_mode);
    
    // Reset counters
    counter1 = 0;
    counter2 = 0;
    
    // Create thread 1
    pthread_create(&thread1, NULL, thread_function_1, NULL);
    
    // Create thread 2 based on selected mode
    switch (run_mode) {
        case 1:
            pthread_create(&thread2, NULL, thread_function_2_safe, NULL);
            break;
        case 2:
            printf("\nWARNING: Deadlock mode selected. The program may hang.\n");
            printf("If it hangs, you'll need to terminate it manually.\n\n");
            pthread_create(&thread2, NULL, thread_function_2_deadlock, NULL);
            break;
        case 3:
            pthread_create(&thread2, NULL, thread_function_2_trylock, NULL);
            break;
        default:
            printf("Invalid mode. Defaulting to safe mode.\n");
            pthread_create(&thread2, NULL, thread_function_2_safe, NULL);
    }
    
    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nFinal counter values: counter1=%d, counter2=%d\n", counter1, counter2);
    
    // Cleanup
    pthread_mutex_destroy(&mutex_A);
    pthread_mutex_destroy(&mutex_B);
    
    return 0;
}