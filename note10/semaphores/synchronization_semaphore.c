#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/**
 * synchronization_semaphore.c
 *
 * This program demonstrates how semaphores can be used for thread
 * synchronization and signaling between threads. It implements
 * a sequence of operations that must occur in a specific order.
 */

#define NUM_STEPS 4

// Semaphores for ordering steps
sem_t step_semaphores[NUM_STEPS];

// Thread function signatures
void* step1_thread(void* arg);
void* step2_thread(void* arg);
void* step3_thread(void* arg);
void* step4_thread(void* arg);

int main() {
    pthread_t threads[NUM_STEPS];
    
    printf("Synchronization Semaphore Demonstration\n");
    printf("--------------------------------------\n");
    printf("This program demonstrates a sequence of operations\n");
    printf("that must occur in a specific order: 1 -> 2 -> 3 -> 4\n\n");
    
    // Initialize all semaphores with value 0
    for (int i = 0; i < NUM_STEPS; i++) {
        if (sem_init(&step_semaphores[i], 0, 0) != 0) {
            perror("sem_init");
            return 1;
        }
    }
    
    // Create threads in reverse order (4,3,2,1)
    // Each thread will wait for its signal from the previous step
    pthread_create(&threads[3], NULL, step4_thread, NULL);
    pthread_create(&threads[2], NULL, step3_thread, NULL);
    pthread_create(&threads[1], NULL, step2_thread, NULL);
    pthread_create(&threads[0], NULL, step1_thread, NULL);
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_STEPS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nAll steps completed in order!\n");
    
    // Destroy all semaphores
    for (int i = 0; i < NUM_STEPS; i++) {
        sem_destroy(&step_semaphores[i]);
    }
    
    return 0;
}

// Step 1: Start the sequence
void* step1_thread(void* arg) {
    printf("Step 1: Starting the sequence...\n");
    sleep(1);
    printf("Step 1: Complete!\n");
    
    // Signal step 2 to proceed
    printf("Step 1: Signaling step 2 to proceed\n");
    sem_post(&step_semaphores[0]);
    
    return NULL;
}

// Step 2: Must wait for step 1 to complete
void* step2_thread(void* arg) {
    printf("Step 2: Waiting for step 1 to complete...\n");
    
    // Wait for step 1 to signal
    sem_wait(&step_semaphores[0]);
    
    printf("Step 2: Starting work (step 1 is complete)...\n");
    sleep(2);
    printf("Step 2: Complete!\n");
    
    // Signal step 3 to proceed
    printf("Step 2: Signaling step 3 to proceed\n");
    sem_post(&step_semaphores[1]);
    
    return NULL;
}

// Step 3: Must wait for step 2 to complete
void* step3_thread(void* arg) {
    printf("Step 3: Waiting for step 2 to complete...\n");
    
    // Wait for step 2 to signal
    sem_wait(&step_semaphores[1]);
    
    printf("Step 3: Starting work (steps 1 & 2 are complete)...\n");
    sleep(1);
    printf("Step 3: Complete!\n");
    
    // Signal step 4 to proceed
    printf("Step 3: Signaling step 4 to proceed\n");
    sem_post(&step_semaphores[2]);
    
    return NULL;
}

// Step 4: Must wait for step 3 to complete
void* step4_thread(void* arg) {
    printf("Step 4: Waiting for step 3 to complete...\n");
    
    // Wait for step 3 to signal
    sem_wait(&step_semaphores[2]);
    
    printf("Step 4: Starting work (steps 1, 2 & 3 are complete)...\n");
    sleep(1);
    printf("Step 4: Complete!\n");
    
    return NULL;
}