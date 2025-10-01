#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// Implementation of a ticket lock

// Ticket lock structure
typedef struct {
    int next_ticket;   // Next ticket to be issued
    int now_serving;   // Currently serving ticket
} ticket_lock_t;

// Initialize the ticket lock
void ticket_lock_init(ticket_lock_t *lock) {
    lock->next_ticket = 0;
    lock->now_serving = 0;
}

// Fetch-and-Add atomic operation (simulated)
// In a real implementation, this would use CPU's atomic instructions
int fetch_and_add(int *ptr, int increment) {
    int old = *ptr;
    *ptr = old + increment;
    return old;
}

// Acquire the ticket lock
void ticket_lock_acquire(ticket_lock_t *lock) {
    // Take a ticket
    int my_ticket = fetch_and_add(&lock->next_ticket, 1);
    
    // Wait until our ticket is called
    while (lock->now_serving != my_ticket) {
        // Busy-wait/spin
    }
    
    // We have the lock when our ticket is being served
}

// Release the ticket lock
void ticket_lock_release(ticket_lock_t *lock) {
    // Serve the next ticket
    lock->now_serving++;
}

// Shared resource
int shared_counter = 0;

// Ticket lock for protecting the counter
ticket_lock_t counter_lock;

// Thread function that increments the counter
void* increment_counter(void* arg) {
    int thread_id = *((int*)arg);
    int iterations = 100000;  // Each thread increments 100,000 times
    
    printf("Thread %d starting with iterations: %d\n", thread_id, iterations);
    
    for (int i = 0; i < iterations; i++) {
        // Enter critical section
        ticket_lock_acquire(&counter_lock);
        
        // Increment counter (critical section)
        shared_counter++;
        
        // Exit critical section
        ticket_lock_release(&counter_lock);
    }
    
    printf("Thread %d completed\n", thread_id);
    return NULL;
}

// Simulate system with multiple threads accessing shared counter
int main() {
    pthread_t threads[4];
    int thread_ids[4] = {0, 1, 2, 3};
    int total_iterations = 100000 * 4;  // Total expected increments
    
    // Initialize the ticket lock
    ticket_lock_init(&counter_lock);
    
    printf("Starting ticket lock demonstration with 4 threads\n");
    
    // Create four threads to increment counter
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, increment_counter, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify result
    printf("\nTicket lock results:\n");
    printf("Expected final counter value: %d\n", total_iterations);
    printf("Actual final counter value: %d\n", shared_counter);
    
    if (shared_counter == total_iterations) {
        printf("SUCCESS: Ticket lock provided proper synchronization\n");
    } else {
        printf("FAILURE: Synchronization error detected\n");
    }
    
    return 0;
}