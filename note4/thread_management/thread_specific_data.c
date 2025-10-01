#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Thread-specific data key
pthread_key_t thread_key;

// Destructor function for thread-specific data
void destructor(void *value) {
    free(value);
    printf("Thread-specific data freed\n");
}

// Thread function that uses thread-specific data
void *thread_function(void *arg) {
    int thread_id = *(int*)arg;
    
    // Allocate memory for thread-specific data
    int *data = malloc(sizeof(int));
    if (data == NULL) {
        perror("malloc");
        return NULL;
    }
    
    // Set thread-specific data
    *data = thread_id * 100;  // Some arbitrary value
    pthread_setspecific(thread_key, data);
    
    // Use thread-specific data
    int *tsd = (int*)pthread_getspecific(thread_key);
    printf("Thread %d: Thread-specific data = %d\n", thread_id, *tsd);
    
    // Modify thread-specific data
    *tsd += 50;
    printf("Thread %d: Modified thread-specific data = %d\n", thread_id, *tsd);
    
    // Sleep to simulate work
    sleep(1);
    
    // Access thread-specific data again
    tsd = (int*)pthread_getspecific(thread_key);
    printf("Thread %d: Thread-specific data after work = %d\n", thread_id, *tsd);
    
    // Note: We don't need to free the thread-specific data
    // The destructor will be called automatically when the thread exits
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;
    int id1 = 1, id2 = 2, id3 = 3;
    
    // Create thread-specific data key with destructor
    if (pthread_key_create(&thread_key, destructor) != 0) {
        perror("pthread_key_create");
        return 1;
    }
    
    printf("Creating threads with thread-specific data\n");
    
    // Create threads
    pthread_create(&thread1, NULL, thread_function, &id1);
    pthread_create(&thread2, NULL, thread_function, &id2);
    pthread_create(&thread3, NULL, thread_function, &id3);
    
    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    
    // Delete the key
    pthread_key_delete(thread_key);
    
    printf("All threads completed\n");
    return 0;
}