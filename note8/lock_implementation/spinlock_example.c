#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

// Implementation of a spinlock

// Spinlock structure
typedef struct {
    int flag;  // 0: unlocked, 1: locked
} spinlock_t;

// Initialize the spinlock
void spinlock_init(spinlock_t *lock) {
    lock->flag = 0;
}

// Test-and-Set atomic operation (simulated in user space)
// In a real implementation, this would use CPU's atomic instructions
int test_and_set(int *target, int value) {
    int old = *target;
    *target = value;
    return old;
}

// Acquire the spinlock
void spinlock_lock(spinlock_t *lock) {
    // Keep trying until we get the lock
    while (test_and_set(&lock->flag, 1) == 1) {
        // Spin - do nothing
    }
}

// Release the spinlock
void spinlock_unlock(spinlock_t *lock) {
    lock->flag = 0;
}

// Shared counter variable
int counter = 0;

// Spinlock for protecting counter
spinlock_t counter_lock;

// Thread function that increments counter
void* increment_counter(void* arg) {
    int i;
    int iterations = *((int*)arg);
    
    for (i = 0; i < iterations; i++) {
        spinlock_lock(&counter_lock);
        counter++;
        spinlock_unlock(&counter_lock);
    }
    
    return NULL;
}

// Thread function that increments counter with exponential backoff
void* increment_with_backoff(void* arg) {
    int i;
    int iterations = *((int*)arg);
    int backoff_time = 1; // microseconds
    
    for (i = 0; i < iterations; i++) {
        // Try to acquire lock
        while (test_and_set(&counter_lock.flag, 1) == 1) {
            // If lock acquisition failed, back off
            usleep(backoff_time);
            
            // Exponential backoff - double the wait time
            // with a maximum cap to prevent excessive waiting
            backoff_time = backoff_time * 2;
            if (backoff_time > 1000) // cap at 1ms
                backoff_time = 1000;
        }
        
        // We got the lock - increment counter
        counter++;
        
        // Release lock
        spinlock_unlock(&counter_lock);
        
        // Reset backoff time for next iteration
        backoff_time = 1;
    }
    
    return NULL;
}

// Simulate high contention with multiple threads
int main() {
    pthread_t threads[4];
    int iterations = 100000;  // Each thread increments 100,000 times
    
    // Initialize the spinlock
    spinlock_init(&counter_lock);
    
    printf("Starting spinlock test with %d threads...\n", 4);
    printf("Each thread will increment the counter %d times\n", iterations);
    
    // Create four threads to increment counter using basic spinlock
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, increment_counter, &iterations);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Basic spinlock: Expected counter value: %d, Actual: %d\n", 
           4 * iterations, counter);
    
    // Reset counter for next test
    counter = 0;
    
    printf("\nStarting spinlock test with exponential backoff...\n");
    
    // Create four threads to increment counter using backoff strategy
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, increment_with_backoff, &iterations);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Spinlock with backoff: Expected counter value: %d, Actual: %d\n", 
           4 * iterations, counter);
    
    return 0;
}