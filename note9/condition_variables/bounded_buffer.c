#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
 * bounded_buffer.c
 * 
 * This program demonstrates a more complex condition variable scenario:
 * a bounded buffer with multiple producer and consumer threads.
 * The program shows the proper use of condition variables to solve
 * the classic producer-consumer problem with a circular buffer.
 */

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 6
#define ITEMS_PER_CONSUMER 9  // 3 producers * 6 items / 2 consumers

// Shared buffer and associated variables
int buffer[BUFFER_SIZE];
int count = 0;      // Number of items in the buffer
int in = 0;         // Index for next producer to add
int out = 0;        // Index for next consumer to remove

// Synchronization primitives
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

// Producer function
void* producer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = (id * 100) + i;  // Create unique item based on producer id
        
        // Acquire lock before checking/modifying shared data
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is full
        while (count == BUFFER_SIZE) {
            printf("Producer %d: Buffer full, waiting...\n", id);
            pthread_cond_wait(&not_full, &mutex);
        }
        
        // Add the item to the buffer
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        printf("Producer %d: Produced item %d (count=%d)\n", id, item, count);
        
        // Signal that the buffer is not empty
        pthread_cond_signal(&not_empty);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Simulate variable production time
        usleep((rand() % 300) * 1000);
    }
    
    printf("Producer %d: Finished producing all items\n", id);
    return NULL;
}

// Consumer function
void* consumer(void* arg) {
    int id = *((int*)arg);
    
    for (int i = 0; i < ITEMS_PER_CONSUMER; i++) {
        // Acquire lock before checking/modifying shared data
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is empty
        while (count == 0) {
            printf("Consumer %d: Buffer empty, waiting...\n", id);
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        // Remove an item from the buffer
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        printf("Consumer %d: Consumed item %d (count=%d)\n", id, item, count);
        
        // Signal that the buffer is not full
        pthread_cond_signal(&not_full);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Simulate variable consumption time
        usleep((rand() % 500) * 1000);
    }
    
    printf("Consumer %d: Finished consuming all items\n", id);
    return NULL;
}

int main() {
    // Thread identifiers
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    
    // Thread IDs to pass to threads
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Bounded Buffer Problem - Condition Variables Demonstration\n");
    printf("-------------------------------------------------------\n");
    printf("Buffer size: %d, Producers: %d, Consumers: %d\n", 
           BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS);
    printf("Each producer creates %d items, each consumer consumes %d items\n",
           ITEMS_PER_PRODUCER, ITEMS_PER_CONSUMER);
    printf("-------------------------------------------------------\n\n");
    
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
    
    // Wait for all producers to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for all consumers to finish
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    printf("\n-------------------------------------------------------\n");
    printf("All threads completed successfully.\n");
    
    // Clean up synchronization primitives
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    
    return 0;
}