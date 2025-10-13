#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
 * condition_variable_demo.c
 * 
 * This program demonstrates condition variables as a synchronization 
 * mechanism between threads. It implements a simple producer-consumer
 * pattern with a single shared buffer slot.
 */

// Shared data
int buffer = -1;
int buffer_has_item = 0;  // Condition: buffer has an item (0=false, 1=true)

// Synchronization primitives
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;  // Signal when buffer becomes empty
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;   // Signal when buffer becomes full

// Producer thread function
void* producer(void* arg) {
    int items_to_produce = *((int*)arg);
    int item;
    
    for (int i = 0; i < items_to_produce; i++) {
        // Generate a value to produce
        item = i + 1;
        
        // Lock the mutex before checking/modifying shared data
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is full (has an item)
        while (buffer_has_item) {
            printf("Producer: Buffer full, waiting for consumer...\n");
            // Release mutex and wait on the 'empty' condition variable
            // When woken up, automatically reacquires mutex
            pthread_cond_wait(&empty, &mutex);
        }
        
        // Buffer is now empty, produce an item
        buffer = item;
        buffer_has_item = 1;
        
        printf("Producer: Produced item %d\n", item);
        
        // Signal to any waiting consumers that the buffer is now full
        pthread_cond_signal(&fill);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Simulate some work being done
        usleep(rand() % 100000);
    }
    
    printf("Producer: Finished producing\n");
    return NULL;
}

// Consumer thread function
void* consumer(void* arg) {
    int items_to_consume = *((int*)arg);
    int item;
    
    for (int i = 0; i < items_to_consume; i++) {
        // Lock the mutex before checking/modifying shared data
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is empty (has no item)
        while (!buffer_has_item) {
            printf("Consumer: Buffer empty, waiting for producer...\n");
            // Release mutex and wait on the 'fill' condition variable
            // When woken up, automatically reacquires mutex
            pthread_cond_wait(&fill, &mutex);
        }
        
        // Buffer has an item, consume it
        item = buffer;
        buffer = -1;
        buffer_has_item = 0;
        
        printf("Consumer: Consumed item %d\n", item);
        
        // Signal to any waiting producers that the buffer is now empty
        pthread_cond_signal(&empty);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Simulate some work being done with the consumed item
        usleep(rand() % 200000);
    }
    
    printf("Consumer: Finished consuming\n");
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;
    int producer_items = 10;
    int consumer_items = 10;
    
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Condition Variable Demonstration - Producer/Consumer Problem\n");
    printf("--------------------------------------------------------\n");
    
    // Create the producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, &producer_items);
    pthread_create(&consumer_thread, NULL, consumer, &consumer_items);
    
    // Wait for both threads to complete
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    printf("--------------------------------------------------------\n");
    printf("All threads completed successfully.\n");
    
    // Clean up resources
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&fill);
    
    return 0;
}