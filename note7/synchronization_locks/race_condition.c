# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>

/*
 * race_condition.c - Demonstrates race conditions and mutex-based solutions
 * 
 * This program shows a shared counter being incremented by multiple threads,
 * both with and without proper synchronization.
 */

#define NUM_THREADS 4
#define INCREMENT_COUNT 1000000

// Global shared counter
volatile int counter = 0;

// Mutex for protecting the counter
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function for unsynchronized increment
void *increment_without_lock(void *arg) {
    int id = *(int*)arg;
    printf("Thread %d starting unsynchronized increments\n", id);
    
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        // This is where the race condition happens
        counter = counter + 1;
    }
    
    printf("Thread %d finished unsynchronized increments\n", id);
    return NULL;
}

// Function for mutex-protected increment
void *increment_with_lock(void *arg) {
    int id = *(int*)arg;
    printf("Thread %d starting synchronized increments\n", id);
    
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        // Use mutex to protect the critical section
        pthread_mutex_lock(&counter_mutex);
        counter = counter + 1;
        pthread_mutex_unlock(&counter_mutex);
    }
    
    printf("Thread %d finished synchronized increments\n", id);
    return NULL;
}

// Function to demonstrate the spinlock implementation
typedef struct {
    volatile int flag;
} spinlock_t;

void spinlock_init(spinlock_t *lock) {
    lock->flag = 0;
}

void spinlock_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(&lock->flag, 1)) {
        // Spin waiting for the lock
        while (lock->flag) {
            // CPU hint to reduce contention (would be PAUSE instruction on x86)
        }
    }
}

void spinlock_unlock(spinlock_t *lock) {
    __sync_lock_release(&lock->flag);
}

// Global spinlock
spinlock_t spin_lock;

// Function for spinlock-protected increment
void *increment_with_spinlock(void *arg) {
    int id = *(int*)arg;
    printf("Thread %d starting spinlock increments\n", id);
    
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        spinlock_lock(&spin_lock);
        counter = counter + 1;
        spinlock_unlock(&spin_lock);
    }
    
    printf("Thread %d finished spinlock increments\n", id);
    return NULL;
}

// Function to demonstrate the ticket lock implementation
typedef struct {
    volatile unsigned int ticket;
    volatile unsigned int serving;
} ticket_lock_t;

void ticket_lock_init(ticket_lock_t *lock) {
    lock->ticket = 0;
    lock->serving = 0;
}

void ticket_lock_lock(ticket_lock_t *lock) {
    unsigned int my_ticket = __sync_fetch_and_add(&lock->ticket, 1);
    
    while (lock->serving != my_ticket) {
        // Spin waiting for my turn
    }
}

void ticket_lock_unlock(ticket_lock_t *lock) {
    __sync_fetch_and_add(&lock->serving, 1);
}

// Global ticket lock
ticket_lock_t ticket_lock;

// Function for ticket lock-protected increment
void *increment_with_ticket_lock(void *arg) {
    int id = *(int*)arg;
    printf("Thread %d starting ticket lock increments\n", id);
    
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        ticket_lock_lock(&ticket_lock);
        counter = counter + 1;
        ticket_lock_unlock(&ticket_lock);
    }
    
    printf("Thread %d finished ticket lock increments\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("Race Condition and Synchronization Demo\n");
    printf("======================================\n\n");
    
    // PART 1: Demonstrate race condition
    printf("Part 1: Demonstrating Race Condition\n\n");
    
    counter = 0;  // Reset counter
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, increment_without_lock, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal counter value (without synchronization): %d\n", counter);
    printf("Expected value (without race): %d\n", NUM_THREADS * INCREMENT_COUNT);
    printf("Difference due to race conditions: %d\n\n", 
           NUM_THREADS * INCREMENT_COUNT - counter);
    
    
    // PART 2: Use mutex for proper synchronization
    printf("Part 2: Using Mutex for Synchronization\n\n");
    
    counter = 0;  // Reset counter
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, increment_with_lock, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal counter value (with mutex): %d\n", counter);
    printf("Expected value: %d\n", NUM_THREADS * INCREMENT_COUNT);
    
    
    // PART 3: Use spinlock for synchronization
    printf("\nPart 3: Using Spinlock for Synchronization\n\n");
    
    counter = 0;  // Reset counter
    spinlock_init(&spin_lock);  // Initialize spinlock
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, increment_with_spinlock, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal counter value (with spinlock): %d\n", counter);
    printf("Expected value: %d\n", NUM_THREADS * INCREMENT_COUNT);
    
    
    // PART 4: Use ticket lock for fair synchronization
    printf("\nPart 4: Using Ticket Lock for Fair Synchronization\n\n");
    
    counter = 0;  // Reset counter
    ticket_lock_init(&ticket_lock);  // Initialize ticket lock
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, increment_with_ticket_lock, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal counter value (with ticket lock): %d\n", counter);
    printf("Expected value: %d\n", NUM_THREADS * INCREMENT_COUNT);
    
    printf("\nConclusion:\n");
    printf("1. Without synchronization: Race conditions cause incorrect results\n");
    printf("2. With mutex: Correct results, OS-assisted sleeping when contended\n");
    printf("3. With spinlock: Correct results, active waiting (efficient for short critical sections)\n");
    printf("4. With ticket lock: Correct results with FIFO ordering (no starvation)\n");
    
    return 0;
}