#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * This program demonstrates condition variables for thread synchronization.
 * It implements a bounded buffer (producer-consumer) pattern using pthread 
 * condition variables to coordinate between producer and consumer threads.
 */

#define BUFFER_SIZE 5

// Shared buffer and associated tracking variables
int buffer[BUFFER_SIZE];
int buffer_count = 0;
int buffer_in = 0;
int buffer_out = 0;

// Synchronization primitives
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;

// Producer function: produces items and adds them to the buffer
void* producer(void* arg) {
    int producer_id = *((int*)arg);
    int item;
    
    for (int i = 0; i < 20; i++) {
        // Generate a random item
        item = rand() % 100;
        
        // Acquire the mutex
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is full
        while (buffer_count == BUFFER_SIZE) {
            printf("Producer %d: Buffer full, waiting...\n", producer_id);
            pthread_cond_wait(&buffer_not_full, &mutex);
        }
        
        // Add item to buffer
        buffer[buffer_in] = item;
        buffer_in = (buffer_in + 1) % BUFFER_SIZE;
        buffer_count++;
        
        printf("Producer %d: Produced item %d, buffer count: %d\n", 
               producer_id, item, buffer_count);
        
        // Signal that buffer is not empty anymore
        pthread_cond_signal(&buffer_not_empty);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Sleep for a short while to simulate varying production rates
        usleep(rand() % 100000);
    }
    
    printf("Producer %d: Finished producing items\n", producer_id);
    return NULL;
}

// Consumer function: takes items from the buffer and consumes them
void* consumer(void* arg) {
    int consumer_id = *((int*)arg);
    int item;
    
    for (int i = 0; i < 20; i++) {
        // Acquire the mutex
        pthread_mutex_lock(&mutex);
        
        // Wait while the buffer is empty
        while (buffer_count == 0) {
            printf("Consumer %d: Buffer empty, waiting...\n", consumer_id);
            pthread_cond_wait(&buffer_not_empty, &mutex);
        }
        
        // Take item from buffer
        item = buffer[buffer_out];
        buffer_out = (buffer_out + 1) % BUFFER_SIZE;
        buffer_count--;
        
        printf("Consumer %d: Consumed item %d, buffer count: %d\n", 
               consumer_id, item, buffer_count);
        
        // Signal that buffer is not full anymore
        pthread_cond_signal(&buffer_not_full);
        
        // Release the mutex
        pthread_mutex_unlock(&mutex);
        
        // Sleep for a short while to simulate varying consumption rates
        usleep(rand() % 100000);
    }
    
    printf("Consumer %d: Finished consuming items\n", consumer_id);
    return NULL;
}

int main() {
    pthread_t producers[2], consumers[2];
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};
    
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Starting producer-consumer demonstration using condition variables\n");
    printf("Buffer size: %d\n", BUFFER_SIZE);
    printf("Each producer and consumer will process 20 items\n\n");
    
    // Create producer threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }
    
    // Create consumer threads
    for (int i = 0; i < 2; i++) {
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }
    
    // Wait for producers to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumers to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    printf("\nAll threads have completed\n");
    
    // Cleanup synchronization primitives
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);
    
    return 0;
}