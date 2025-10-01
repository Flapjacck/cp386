#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Reader-Writer lock structure
typedef struct {
    pthread_mutex_t mutex;       // Mutex for protecting reader count
    pthread_mutex_t write_lock;  // Mutex for writers
    int readers;                 // Number of active readers
} rw_lock_t;

// Initialize reader-writer lock
void rw_lock_init(rw_lock_t *lock) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_mutex_init(&lock->write_lock, NULL);
    lock->readers = 0;
}

// Clean up reader-writer lock
void rw_lock_destroy(rw_lock_t *lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_mutex_destroy(&lock->write_lock);
}

// Acquire read lock
void rw_read_lock(rw_lock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->readers++;
    if (lock->readers == 1) {
        // First reader locks out writers
        pthread_mutex_lock(&lock->write_lock);
    }
    pthread_mutex_unlock(&lock->mutex);
}

// Release read lock
void rw_read_unlock(rw_lock_t *lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->readers--;
    if (lock->readers == 0) {
        // Last reader allows writers
        pthread_mutex_unlock(&lock->write_lock);
    }
    pthread_mutex_unlock(&lock->mutex);
}

// Acquire write lock
void rw_write_lock(rw_lock_t *lock) {
    pthread_mutex_lock(&lock->write_lock);
}

// Release write lock
void rw_write_unlock(rw_lock_t *lock) {
    pthread_mutex_unlock(&lock->write_lock);
}

// Shared data
int shared_data = 0;
rw_lock_t rw_lock;

// Reader thread function
void *reader(void *arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < 5; i++) {
        // Simulate random work before reading
        usleep(rand() % 100000);
        
        // Acquire read lock
        rw_read_lock(&rw_lock);
        
        // Read data (multiple readers can do this simultaneously)
        printf("Reader %d: reading data = %d\n", id, shared_data);
        
        // Simulate time spent reading
        usleep(rand() % 100000);
        
        // Release read lock
        rw_read_unlock(&rw_lock);
    }
    
    return NULL;
}

// Writer thread function
void *writer(void *arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < 3; i++) {
        // Simulate random work before writing
        usleep(rand() % 200000);
        
        // Acquire write lock (exclusive access)
        rw_write_lock(&rw_lock);
        
        // Modify data
        shared_data++;
        printf("Writer %d: updated data to %d\n", id, shared_data);
        
        // Simulate time spent writing
        usleep(rand() % 100000);
        
        // Release write lock
        rw_write_unlock(&rw_lock);
    }
    
    return NULL;
}

int main() {
    pthread_t readers[5];
    pthread_t writers[2];
    int reader_ids[5];
    int writer_ids[2];
    
    // Seed random number generator
    srand(time(NULL));
    
    // Initialize reader-writer lock
    rw_lock_init(&rw_lock);
    
    printf("Starting reader-writer demonstration\n");
    
    // Create reader threads
    for (int i = 0; i < 5; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    // Create writer threads
    for (int i = 0; i < 2; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    // Wait for reader threads to finish
    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }
    
    // Wait for writer threads to finish
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // Clean up
    rw_lock_destroy(&rw_lock);
    
    printf("Final shared data value: %d\n", shared_data);
    printf("Reader-writer demonstration completed\n");
    
    return 0;
}