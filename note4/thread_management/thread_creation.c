#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Thread function to demonstrate basic thread creation
void *thread_function(void *arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d is running\n", thread_id);
    sleep(1);  // Simulate some work
    printf("Thread %d is finished\n", thread_id);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;
    
    printf("Main thread: Creating threads\n");
    
    // Create two threads
    if (pthread_create(&thread1, NULL, thread_function, &id1) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    if (pthread_create(&thread2, NULL, thread_function, &id2) != 0) {
        perror("pthread_create");
        return 1;
    }
    
    printf("Main thread: Waiting for threads to finish\n");
    
    // Wait for threads to finish
    if (pthread_join(thread1, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }
    
    if (pthread_join(thread2, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }
    
    printf("Main thread: All threads have completed\n");
    return 0;
}