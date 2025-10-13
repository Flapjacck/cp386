#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * counting_semaphore.c
 *
 * This program demonstrates the use of a counting semaphore to limit
 * access to a fixed set of resources. It simulates a resource pool
 * where only N threads can access the resource simultaneously.
 */

#define NUM_THREADS 10      // Total number of threads
#define RESOURCE_LIMIT 3    // Only 3 threads can use the resource at once
#define MAX_WORK_TIME 3     // Maximum time a thread will hold the resource (seconds)

// Semaphore for the resource pool
sem_t resource_pool;

// Mutex for protecting console output
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to print thread-safe messages
void safe_print(const char* format, ...) {
    va_list args;
    pthread_mutex_lock(&print_mutex);
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    pthread_mutex_unlock(&print_mutex);
}

// Worker thread function
void* worker(void* arg) {
    int id = *((int*)arg);
    
    safe_print("Thread %d: Trying to acquire resource...\n", id);
    
    // Attempt to acquire a resource (blocks if none available)
    sem_wait(&resource_pool);
    
    // Resource acquired, get current count
    int value;
    sem_getvalue(&resource_pool, &value);
    
    safe_print("Thread %d: Resource acquired! Resources still available: %d\n", 
               id, value);
    
    // Simulate using the resource for some time
    int work_time = (rand() % MAX_WORK_TIME) + 1;
    safe_print("Thread %d: Using resource for %d seconds...\n", id, work_time);
    sleep(work_time);
    
    // Release the resource
    sem_post(&resource_pool);
    
    // Get updated count
    sem_getvalue(&resource_pool, &value);
    safe_print("Thread %d: Resource released. Resources now available: %d\n", 
               id, value);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Counting Semaphore Demonstration (Resource Pool)\n");
    printf("-----------------------------------------------\n");
    printf("Total threads: %d\n", NUM_THREADS);
    printf("Resource limit: %d\n", RESOURCE_LIMIT);
    printf("-----------------------------------------------\n\n");
    
    // Initialize the semaphore with RESOURCE_LIMIT (counting semaphore)
    if (sem_init(&resource_pool, 0, RESOURCE_LIMIT) != 0) {
        perror("sem_init");
        return 1;
    }
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, worker, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
        
        // Small delay between thread creation to make output more readable
        usleep(100000);  // 100ms
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nAll threads have completed.\n");
    
    // Destroy the semaphore
    sem_destroy(&resource_pool);
    pthread_mutex_destroy(&print_mutex);
    
    return 0;
}