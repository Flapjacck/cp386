#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 10

// Shared buffer
typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} buffer_t;

// Initialize buffer
void buffer_init(buffer_t *buffer) {
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
}

// Clean up buffer resources
void buffer_destroy(buffer_t *buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->not_full);
    pthread_cond_destroy(&buffer->not_empty);
}

// Producer thread function
void *producer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Simulate production time
        usleep(rand() % 100000);
        
        // Produce item
        int item = i + 1;
        
        // Acquire lock
        pthread_mutex_lock(&buffer->mutex);
        
        // Wait if buffer is full
        while (buffer->count == BUFFER_SIZE) {
            printf("Producer: Buffer full, waiting...\n");
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        
        // Add item to buffer
        buffer->buffer[buffer->in] = item;
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        buffer->count++;
        
        printf("Producer: Inserted item %d into buffer\n", item);
        
        // Signal that buffer is not empty
        pthread_cond_signal(&buffer->not_empty);
        
        // Release lock
        pthread_mutex_unlock(&buffer->mutex);
    }
    
    printf("Producer: Finished producing all items\n");
    return NULL;
}

// Consumer thread function
void *consumer(void *arg) {
    buffer_t *buffer = (buffer_t*)arg;
    
    for (int i = 0; i < NUM_ITEMS; i++) {
        // Acquire lock
        pthread_mutex_lock(&buffer->mutex);
        
        // Wait if buffer is empty
        while (buffer->count == 0) {
            printf("Consumer: Buffer empty, waiting...\n");
            pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
        }
        
        // Remove item from buffer
        int item = buffer->buffer[buffer->out];
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;
        buffer->count--;
        
        printf("Consumer: Removed item %d from buffer\n", item);
        
        // Signal that buffer is not full
        pthread_cond_signal(&buffer->not_full);
        
        // Release lock
        pthread_mutex_unlock(&buffer->mutex);
        
        // Simulate consumption time
        usleep(rand() % 200000);
    }
    
    printf("Consumer: Finished consuming all items\n");
    return NULL;
}

int main() {
    buffer_t buffer;
    pthread_t producer_thread, consumer_thread;
    
    // Seed random number generator
    srand(time(NULL));
    
    // Initialize buffer
    buffer_init(&buffer);
    
    printf("Starting producer-consumer demonstration\n");
    
    // Create producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, &buffer);
    pthread_create(&consumer_thread, NULL, consumer, &buffer);
    
    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    // Clean up
    buffer_destroy(&buffer);
    
    printf("Producer-consumer demonstration completed\n");
    return 0;
}