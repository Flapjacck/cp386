#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * producer_consumer_semaphores.c
 *
 * This program implements the classic producer-consumer problem using
 * semaphores rather than condition variables. It demonstrates how
 * three semaphores can effectively coordinate access to a bounded buffer.
 */

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 3
#define ITEMS_PER_PRODUCER 8
#define TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

// Shared buffer and indices
int buffer[BUFFER_SIZE];
int buffer_index = 0;

// Semaphores for synchronization
sem_t empty;     // Count of empty buffer slots (initially BUFFER_SIZE)
sem_t full;      // Count of filled buffer slots (initially 0)
sem_t mutex;     // Binary semaphore for mutual exclusion (initially 1)

// Track the total number of items consumed for program termination
int total_consumed = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Producer function
void* producer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        // Create an item
        int item = (id * 100) + i;
        
        // Wait for an empty slot
        sem_wait(&empty);
        
        // Wait for exclusive access to the buffer
        sem_wait(&mutex);
        
        // Add the item to the buffer
        buffer[buffer_index] = item;
        printf("Producer %d: Produced item %d at position %d\n", 
               id, item, buffer_index);
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;
        
        // Release exclusive access
        sem_post(&mutex);
        
        // Signal that a new item is available
        sem_post(&full);
        
        // Random production delay
        usleep((rand() % 500) * 1000);
    }
    
    printf("Producer %d: Finished producing all items\n", id);
    return NULL;
}

// Consumer function
void* consumer(void* arg) {
    int id = *((int*)arg);
    int items_consumed = 0;
    int should_continue = 1;
    
    while (should_continue) {
        // Wait for an item to be available
        sem_wait(&full);
        
        // Wait for exclusive access to the buffer
        sem_wait(&mutex);
        
        // Consume the item
        int item = buffer[(buffer_index - 1 + BUFFER_SIZE) % BUFFER_SIZE];
        buffer_index = (buffer_index - 1 + BUFFER_SIZE) % BUFFER_SIZE;
        printf("Consumer %d: Consumed item %d from position %d\n", 
               id, item, buffer_index);
        
        // Release exclusive access
        sem_post(&mutex);
        
        // Signal that an empty slot is available
        sem_post(&empty);
        
        // Increment local and global consumed counts
        items_consumed++;
        
        // Update total count safely
        pthread_mutex_lock(&count_mutex);
        total_consumed++;
        
        // Check if we've consumed all expected items
        if (total_consumed >= TOTAL_ITEMS) {
            should_continue = 0;
        }
        pthread_mutex_unlock(&count_mutex);
        
        // Random consumption delay
        usleep((rand() % 800) * 1000);
    }
    
    printf("Consumer %d: Consumed %d items\n", id, items_consumed);
    return NULL;
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Producer-Consumer Problem Using Semaphores\n");
    printf("-----------------------------------------\n");
    printf("Buffer size: %d\n", BUFFER_SIZE);
    printf("Producers: %d, Items per producer: %d\n", NUM_PRODUCERS, ITEMS_PER_PRODUCER);
    printf("Consumers: %d, Total items: %d\n", NUM_CONSUMERS, TOTAL_ITEMS);
    printf("-----------------------------------------\n\n");
    
    // Initialize semaphores
    sem_init(&empty, 0, BUFFER_SIZE);  // Initially all slots are empty
    sem_init(&full, 0, 0);             // Initially no items are available
    sem_init(&mutex, 0, 1);            // Binary semaphore for mutual exclusion
    
    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }
    
    // Wait for producers to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumers to finish
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    printf("\n-----------------------------------------\n");
    printf("All threads completed. Total items produced/consumed: %d\n", TOTAL_ITEMS);
    
    // Cleanup
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    pthread_mutex_destroy(&count_mutex);
    
    return 0;
}