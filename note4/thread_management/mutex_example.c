#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Shared data structure
typedef struct {
    int counter;
    pthread_mutex_t mutex;
} shared_data_t;

// Thread function that increments the counter with a mutex
void *safe_increment(void *arg) {
    shared_data_t *data = (shared_data_t*)arg;
    
    for (int i = 0; i < 100000; i++) {
        // Lock the mutex before accessing shared data
        pthread_mutex_lock(&data->mutex);
        
        // Critical section - only one thread can be here at a time
        data->counter++;
        
        // Unlock the mutex
        pthread_mutex_unlock(&data->mutex);
    }
    
    return NULL;
}

// Thread function that increments the counter without synchronization
void *unsafe_increment(void *arg) {
    shared_data_t *data = (shared_data_t*)arg;
    
    for (int i = 0; i < 100000; i++) {
        // No synchronization - race condition can occur
        data->counter++;
    }
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    shared_data_t safe_data = {0, PTHREAD_MUTEX_INITIALIZER};
    shared_data_t unsafe_data = {0, PTHREAD_MUTEX_INITIALIZER};
    
    printf("Starting mutex demonstration\n");
    
    // Safe increment with mutex
    printf("Safe counter starting value: %d\n", safe_data.counter);
    
    pthread_create(&thread1, NULL, safe_increment, &safe_data);
    pthread_create(&thread2, NULL, safe_increment, &safe_data);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Safe counter final value: %d (expected: 200000)\n", safe_data.counter);
    
    // Unsafe increment without mutex
    printf("\nUnsafe counter starting value: %d\n", unsafe_data.counter);
    
    pthread_create(&thread1, NULL, unsafe_increment, &unsafe_data);
    pthread_create(&thread2, NULL, unsafe_increment, &unsafe_data);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Unsafe counter final value: %d (expected: 200000)\n", unsafe_data.counter);
    printf("If value is less than 200000, a race condition occurred\n");
    
    // Clean up mutex
    pthread_mutex_destroy(&safe_data.mutex);
    pthread_mutex_destroy(&unsafe_data.mutex);
    
    return 0;
}