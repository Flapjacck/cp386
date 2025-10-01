# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>

/*
 * deadlock.c - Demonstrates a deadlock scenario and prevention
 * 
 * This program shows how deadlocks can occur when two threads
 * acquire locks in different orders.
 */

// Global mutexes
pthread_mutex_t mutex_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_B = PTHREAD_MUTEX_INITIALIZER;

// Global resources
int resource_A = 100;
int resource_B = 200;

// Thread function that acquires locks in order A->B (can deadlock)
void *thread_1_function(void *arg) {
    printf("Thread 1: Trying to acquire mutex A\n");
    pthread_mutex_lock(&mutex_A);
    printf("Thread 1: Acquired mutex A\n");
    
    // Simulate some work
    printf("Thread 1: Working with resource A\n");
    resource_A += 10;
    sleep(1);  // This delay increases the chance of deadlock
    
    printf("Thread 1: Trying to acquire mutex B\n");
    pthread_mutex_lock(&mutex_B);
    printf("Thread 1: Acquired mutex B\n");
    
    // Critical section with both resources
    printf("Thread 1: Working with both resources\n");
    int result = resource_A + resource_B;
    printf("Thread 1: Result = %d\n", result);
    
    // Release locks in reverse order
    pthread_mutex_unlock(&mutex_B);
    printf("Thread 1: Released mutex B\n");
    
    pthread_mutex_unlock(&mutex_A);
    printf("Thread 1: Released mutex A\n");
    
    return NULL;
}

// Thread function that acquires locks in order B->A (can deadlock)
void *thread_2_function(void *arg) {
    printf("Thread 2: Trying to acquire mutex B\n");
    pthread_mutex_lock(&mutex_B);
    printf("Thread 2: Acquired mutex B\n");
    
    // Simulate some work
    printf("Thread 2: Working with resource B\n");
    resource_B += 20;
    sleep(1);  // This delay increases the chance of deadlock
    
    printf("Thread 2: Trying to acquire mutex A\n");
    pthread_mutex_lock(&mutex_A);
    printf("Thread 2: Acquired mutex A\n");
    
    // Critical section with both resources
    printf("Thread 2: Working with both resources\n");
    int result = resource_B - resource_A;
    printf("Thread 2: Result = %d\n", result);
    
    // Release locks in reverse order
    pthread_mutex_unlock(&mutex_A);
    printf("Thread 2: Released mutex A\n");
    
    pthread_mutex_unlock(&mutex_B);
    printf("Thread 2: Released mutex B\n");
    
    return NULL;
}

// Thread function that acquires locks in consistent order A->B (no deadlock)
void *thread_2_safe_function(void *arg) {
    printf("Thread 2 (safe): Trying to acquire mutex A\n");
    pthread_mutex_lock(&mutex_A);
    printf("Thread 2 (safe): Acquired mutex A\n");
    
    // Simulate some work
    printf("Thread 2 (safe): Working with resource A\n");
    resource_A += 5;
    sleep(1);
    
    printf("Thread 2 (safe): Trying to acquire mutex B\n");
    pthread_mutex_lock(&mutex_B);
    printf("Thread 2 (safe): Acquired mutex B\n");
    
    // Critical section with both resources
    printf("Thread 2 (safe): Working with both resources\n");
    int result = resource_B - resource_A;
    printf("Thread 2 (safe): Result = %d\n", result);
    
    // Release locks in reverse order
    pthread_mutex_unlock(&mutex_B);
    printf("Thread 2 (safe): Released mutex B\n");
    
    pthread_mutex_unlock(&mutex_A);
    printf("Thread 2 (safe): Released mutex A\n");
    
    return NULL;
}

// Thread function using trylock to avoid deadlock
void *thread_2_trylock_function(void *arg) {
    int got_both_locks = 0;
    
    while (!got_both_locks) {
        printf("Thread 2 (trylock): Trying to acquire mutex B\n");
        if (pthread_mutex_trylock(&mutex_B) == 0) {
            printf("Thread 2 (trylock): Acquired mutex B\n");
            
            // Simulate some work
            printf("Thread 2 (trylock): Working with resource B\n");
            resource_B += 20;
            
            printf("Thread 2 (trylock): Trying to acquire mutex A\n");
            if (pthread_mutex_trylock(&mutex_A) == 0) {
                printf("Thread 2 (trylock): Acquired mutex A\n");
                
                // We have both locks now
                got_both_locks = 1;
                
                // Critical section with both resources
                printf("Thread 2 (trylock): Working with both resources\n");
                int result = resource_B - resource_A;
                printf("Thread 2 (trylock): Result = %d\n", result);
                
                // Release lock A
                pthread_mutex_unlock(&mutex_A);
                printf("Thread 2 (trylock): Released mutex A\n");
            } else {
                printf("Thread 2 (trylock): Could not acquire mutex A, releasing B and retrying\n");
            }
            
            // Release lock B
            pthread_mutex_unlock(&mutex_B);
            printf("Thread 2 (trylock): Released mutex B\n");
            
            if (!got_both_locks) {
                // Random backoff before retry to reduce contention
                usleep(rand() % 500000);  // Sleep up to 0.5 seconds
            }
        } else {
            printf("Thread 2 (trylock): Could not acquire mutex B, retrying\n");
            // Random backoff before retry
            usleep(rand() % 500000);  // Sleep up to 0.5 seconds
        }
    }
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("Deadlock Demonstration\n");
    printf("=====================\n\n");
    
    printf("PART 1: Deadlock Scenario\n");
    printf("This will likely deadlock and you'll need to terminate the program.\n");
    printf("Press Ctrl+C after about 10 seconds if execution stalls.\n\n");
    
    // Create threads that acquire locks in opposite order (will likely deadlock)
    pthread_create(&thread1, NULL, thread_1_function, NULL);
    pthread_create(&thread2, NULL, thread_2_function, NULL);
    
    // Wait for threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nIf you see this message, deadlock didn't occur (unusual).\n");
    
    // Reset resources
    resource_A = 100;
    resource_B = 200;
    
    printf("\nPART 2: Deadlock Prevention - Consistent Lock Ordering\n\n");
    
    // Create threads that acquire locks in the same order (will not deadlock)
    pthread_create(&thread1, NULL, thread_1_function, NULL);
    pthread_create(&thread2, NULL, thread_2_safe_function, NULL);
    
    // Wait for threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nConsistent lock ordering completed successfully - no deadlock!\n");
    
    // Reset resources
    resource_A = 100;
    resource_B = 200;
    
    printf("\nPART 3: Deadlock Prevention - Using trylock\n\n");
    
    // Create threads with one using trylock (will not deadlock)
    pthread_create(&thread1, NULL, thread_1_function, NULL);
    pthread_create(&thread2, NULL, thread_2_trylock_function, NULL);
    
    // Wait for threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nTrylock approach completed successfully - no deadlock!\n");
    
    printf("\nSummary of Deadlock Prevention Techniques:\n");
    printf("1. Consistent Lock Ordering: Always acquire locks in the same order\n");
    printf("2. Trylock with Backoff: Use non-blocking lock attempts with retry\n");
    printf("3. Lock Timeout: Use timed lock attempts (not shown in this example)\n");
    printf("4. Deadlock Detection: Use a resource allocation graph (system level)\n");
    printf("5. Lock Hierarchy: Design locks with a clear hierarchy that must be followed\n");
    
    // Clean up
    pthread_mutex_destroy(&mutex_A);
    pthread_mutex_destroy(&mutex_B);
    
    return 0;
}