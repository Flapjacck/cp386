#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 3
#define NUM_TASKS 10

// Task structure
typedef struct {
    int task_id;
    void (*function)(int);
} task_t;

// Thread pool structure
typedef struct {
    task_t *task_queue;           // Task queue
    int queue_size;               // Size of queue
    int head, tail, count;        // Queue management
    pthread_mutex_t queue_lock;   // Queue lock
    pthread_cond_t queue_not_empty; // Signal when queue has tasks
    pthread_cond_t queue_not_full;  // Signal when queue has space
    pthread_t *threads;           // Worker threads
    int num_threads;              // Number of threads
    int shutdown;                 // Shutdown flag
} thread_pool_t;

// Function to be executed by the tasks
void task_function(int id) {
    printf("Task %d started\n", id);
    // Simulate work
    usleep((rand() % 1000) * 1000);
    printf("Task %d completed\n", id);
}

// Worker thread function
void *worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    task_t task;
    
    while (1) {
        // Lock the queue
        pthread_mutex_lock(&pool->queue_lock);
        
        // Wait if queue is empty and pool is not shutting down
        while (pool->count == 0 && !pool->shutdown) {
            printf("Thread %lu waiting for work\n", (unsigned long)pthread_self());
            pthread_cond_wait(&pool->queue_not_empty, &pool->queue_lock);
        }
        
        // If pool is shutting down and queue is empty, exit
        if (pool->shutdown && pool->count == 0) {
            pthread_mutex_unlock(&pool->queue_lock);
            printf("Thread %lu exiting\n", (unsigned long)pthread_self());
            pthread_exit(NULL);
        }
        
        // Get a task from the queue
        task = pool->task_queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;
        
        // Signal that queue is not full
        pthread_cond_signal(&pool->queue_not_full);
        
        // Unlock the queue
        pthread_mutex_unlock(&pool->queue_lock);
        
        // Execute the task
        printf("Thread %lu executing task %d\n", (unsigned long)pthread_self(), task.task_id);
        task.function(task.task_id);
    }
    
    return NULL;
}

// Initialize thread pool
thread_pool_t *thread_pool_init(int num_threads, int queue_size) {
    thread_pool_t *pool;
    
    // Allocate pool structure
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    if (pool == NULL) {
        return NULL;
    }
    
    // Initialize pool attributes
    pool->queue_size = queue_size;
    pool->count = 0;
    pool->head = 0;
    pool->tail = 0;
    pool->shutdown = 0;
    pool->num_threads = num_threads;
    
    // Allocate task queue
    pool->task_queue = (task_t *)malloc(sizeof(task_t) * queue_size);
    if (pool->task_queue == NULL) {
        free(pool);
        return NULL;
    }
    
    // Initialize mutex and condition variables
    pthread_mutex_init(&pool->queue_lock, NULL);
    pthread_cond_init(&pool->queue_not_empty, NULL);
    pthread_cond_init(&pool->queue_not_full, NULL);
    
    // Create worker threads
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    if (pool->threads == NULL) {
        free(pool->task_queue);
        free(pool);
        return NULL;
    }
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker, pool) != 0) {
            // Clean up on error
            thread_pool_destroy(pool);
            return NULL;
        }
    }
    
    return pool;
}

// Add a task to the thread pool
int thread_pool_add_task(thread_pool_t *pool, int task_id, void (*function)(int)) {
    // Lock the queue
    pthread_mutex_lock(&pool->queue_lock);
    
    // Wait if queue is full
    while (pool->count == pool->queue_size && !pool->shutdown) {
        pthread_cond_wait(&pool->queue_not_full, &pool->queue_lock);
    }
    
    // Don't add if shutting down
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_lock);
        return -1;
    }
    
    // Add task to queue
    pool->task_queue[pool->tail].task_id = task_id;
    pool->task_queue[pool->tail].function = function;
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->count++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&pool->queue_not_empty);
    
    // Unlock the queue
    pthread_mutex_unlock(&pool->queue_lock);
    
    return 0;
}

// Destroy the thread pool
void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL) {
        return;
    }
    
    // Lock the queue
    pthread_mutex_lock(&pool->queue_lock);
    
    // Set shutdown flag
    pool->shutdown = 1;
    
    // Signal all threads to wake up
    pthread_cond_broadcast(&pool->queue_not_empty);
    pthread_cond_broadcast(&pool->queue_not_full);
    
    // Unlock the queue
    pthread_mutex_unlock(&pool->queue_lock);
    
    // Wait for all threads to finish
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // Free resources
    free(pool->threads);
    free(pool->task_queue);
    pthread_mutex_destroy(&pool->queue_lock);
    pthread_cond_destroy(&pool->queue_not_empty);
    pthread_cond_destroy(&pool->queue_not_full);
    free(pool);
}

int main() {
    // Seed random number generator
    srand(time(NULL));
    
    printf("Initializing thread pool with %d threads\n", NUM_THREADS);
    
    // Initialize thread pool
    thread_pool_t *pool = thread_pool_init(NUM_THREADS, NUM_TASKS);
    if (pool == NULL) {
        printf("Failed to initialize thread pool\n");
        return 1;
    }
    
    printf("Adding tasks to thread pool\n");
    
    // Add tasks to the pool
    for (int i = 0; i < NUM_TASKS; i++) {
        thread_pool_add_task(pool, i, task_function);
        printf("Added task %d to queue\n", i);
    }
    
    // Sleep to allow tasks to be processed
    printf("Main thread sleeping while tasks are processed\n");
    sleep(5);
    
    // Shutdown thread pool
    printf("Shutting down thread pool\n");
    thread_pool_destroy(pool);
    
    printf("Thread pool demonstration completed\n");
    return 0;
}