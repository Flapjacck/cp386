/*
 * ===================================================================
 * Threading and Concurrency Demonstration Program
 * ===================================================================
 * 
 * This program demonstrates fundamental concepts in concurrent programming:
 * race conditions, shared memory, and the need for synchronization. It shows
 * what happens when multiple threads access shared data without proper
 * coordination - a classic example of why thread synchronization is critical.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Race Conditions: Multiple threads competing for shared resources
 * 2. Shared Memory: Global variables accessible by all threads
 * 3. Non-atomic Operations: Operations that can be interrupted mid-execution
 * 4. Indeterminate Results: Output varies between program runs
 * 5. Thread Creation and Management: POSIX pthread library usage
 * 
 * RACE CONDITION MECHANICS:
 * ------------------------
 * A race condition occurs when the correctness of a program depends on the
 * relative timing or interleaving of multiple threads. In this program:
 * 
 * Thread 1: Read counter (0) → Increment (1) → Write counter (1)
 * Thread 2: Read counter (0) → Increment (1) → Write counter (1)
 * 
 * Expected result: counter = 2
 * Actual result: counter = 1 (lost update!)
 * 
 * THE INCREMENT OPERATION ISN'T ATOMIC:
 * ------------------------------------
 * counter++ actually involves multiple CPU instructions:
 * 1. LOAD  counter from memory to register
 * 2. ADD   1 to register value  
 * 3. STORE register value back to memory
 * 
 * If two threads execute these steps simultaneously:
 * Time 1: Thread A loads counter (value 5)
 * Time 2: Thread B loads counter (value 5) 
 * Time 3: Thread A increments register (6)
 * Time 4: Thread B increments register (6)
 * Time 5: Thread A stores 6 to counter
 * Time 6: Thread B stores 6 to counter
 * Result: counter = 6 (should be 7!)
 * 
 * References:
 * - OSTEP Chapter 26: Concurrency and Threads
 * - POSIX Threads Programming (pthreads)
 * - Computer Systems: A Programmer's Perspective (Bryant & O'Hallaron)
 */

#include <stdio.h>       // For printf(), fprintf()
#include <stdlib.h>      // For exit(), atoi(), EXIT_FAILURE, EXIT_SUCCESS
#include <pthread.h>     // For pthread_* functions
#include <unistd.h>      // For sleep(), getpid()
#include <sys/time.h>    // For timing functions
#include <stdint.h>      // For uintptr_t
#include <sched.h>       // For sched_yield()
#include "../../common.h"   // Our common utility functions

/*
 * SHARED GLOBAL VARIABLES
 * =======================
 * These variables are shared among all threads in the process.
 * This shared state is what makes race conditions possible.
 */

// The shared counter that all threads will modify
// volatile tells compiler not to optimize access to this variable
// (it may be modified by other threads)
volatile int counter = 0;

// Number of iterations each thread will perform
int loops;

// Thread performance tracking
volatile double thread1_start_time = 0;
volatile double thread2_start_time = 0;
volatile double thread1_end_time = 0;
volatile double thread2_end_time = 0;

/*
 * WORKER THREAD FUNCTION
 * ======================
 * 
 * This function represents the work that each thread performs.
 * Both threads execute this same function concurrently, leading
 * to race conditions on the shared 'counter' variable.
 * 
 * The function demonstrates:
 * 1. Thread identification
 * 2. Shared memory access
 * 3. Race condition creation
 * 4. Performance measurement
 */
void *worker(void *arg) {
    /*
     * THREAD IDENTIFICATION
     * ====================
     * Get the thread ID for debugging and monitoring purposes.
     * This helps us distinguish between different threads in output.
     */
    pthread_t thread_id = pthread_self();
    int thread_num = (int)(uintptr_t)arg;  // Convert void* back to int
    
    printf("[Thread %d] Starting execution (pthread_id: %lu)\n", 
           thread_num, (unsigned long)thread_id);
    
    // Record start time for performance analysis
    double start_time = GetTime();
    if (thread_num == 1) {
        thread1_start_time = start_time;
    } else {
        thread2_start_time = start_time;
    }
    
    /*
     * THE CRITICAL SECTION
     * ===================
     * 
     * This loop contains the "critical section" - code that accesses
     * shared data and must not be executed by multiple threads simultaneously.
     * 
     * Each iteration performs counter++, which is NOT atomic:
     * 
     * Assembly representation of counter++:
     * 1. mov    eax, [counter]    ; Load counter into register
     * 2. inc    eax               ; Increment register
     * 3. mov    [counter], eax    ; Store back to memory
     * 
     * If thread switching occurs between any of these instructions,
     * race conditions will occur.
     */
    printf("[Thread %d] Entering critical section with %d iterations\n", 
           thread_num, loops);
    
    for (int i = 0; i < loops; i++) {
        /*
         * INTENTIONAL RACE CONDITION
         * =========================
         * 
         * We deliberately access the shared counter without synchronization
         * to demonstrate race conditions. In real code, this section would
         * be protected by:
         * - Mutexes (pthread_mutex_lock/unlock)
         * - Atomic operations (__sync_fetch_and_add)
         * - Semaphores
         * - Other synchronization primitives
         */
        
        // Read current value (potentially interrupted here)
        int temp = counter;
        
        // Optional: Add small delay to increase chance of race condition
        // This makes the race condition more visible for educational purposes
        if (i % 1000 == 0) {
            // Yield CPU to other threads occasionally
            sched_yield();  // Give other threads a chance to run (POSIX standard)
        }
        
        // Increment and write back (race condition window)
        counter = temp + 1;
        
        /*
         * RACE CONDITION ANALYSIS
         * ======================
         * 
         * If both threads read the same value of 'counter' before either
         * writes it back, we get a "lost update":
         * 
         * Scenario 1 (Correct):
         * Thread 1: read(0) → inc(1) → write(1)
         * Thread 2:                   → read(1) → inc(2) → write(2)
         * Result: counter = 2 ✓
         * 
         * Scenario 2 (Race Condition):
         * Thread 1: read(0) → inc(1) → 
         * Thread 2: read(0) → inc(1) → write(1)
         * Thread 1:                  → write(1)
         * Result: counter = 1 ✗ (should be 2)
         */
    }
    
    // Record end time for performance analysis
    double end_time = GetTime();
    if (thread_num == 1) {
        thread1_end_time = end_time;
    } else {
        thread2_end_time = end_time;
    }
    
    printf("[Thread %d] Completed %d iterations in %.6f seconds\n",
           thread_num, loops, end_time - start_time);
    
    return NULL;  // Thread termination
}

/*
 * PERFORMANCE ANALYSIS FUNCTION
 * =============================
 * 
 * Analyzes the timing and efficiency of thread execution.
 * This helps understand thread scheduling and parallelism.
 */
void analyze_performance() {
    printf("\n=== Performance Analysis ===\n");
    
    double thread1_duration = thread1_end_time - thread1_start_time;
    double thread2_duration = thread2_end_time - thread2_start_time;
    double total_start = (thread1_start_time < thread2_start_time) ? 
                        thread1_start_time : thread2_start_time;
    double total_end = (thread1_end_time > thread2_end_time) ? 
                      thread1_end_time : thread2_end_time;
    double total_duration = total_end - total_start;
    
    printf("Thread 1 execution time: %.6f seconds\n", thread1_duration);
    printf("Thread 2 execution time: %.6f seconds\n", thread2_duration);
    printf("Total wall clock time:   %.6f seconds\n", total_duration);
    
    // Calculate theoretical speedup
    double sequential_time = thread1_duration + thread2_duration;
    double speedup = sequential_time / total_duration;
    
    printf("Sequential time estimate: %.6f seconds\n", sequential_time);
    printf("Parallel speedup:         %.2fx\n", speedup);
    
    if (speedup < 1.5) {
        printf("Low speedup indicates contention or overhead\n");
    } else if (speedup > 1.8) {
        printf("Good parallel efficiency!\n");
    }
}

/*
 * RACE CONDITION ANALYSIS FUNCTION
 * ================================
 * 
 * Analyzes the results to determine if race conditions occurred
 * and explains the implications.
 */
void analyze_race_condition(int expected, int actual) {
    printf("\n=== Race Condition Analysis ===\n");
    printf("Expected final value: %d\n", expected);
    printf("Actual final value:   %d\n", actual);
    
    if (actual == expected) {
        printf("✓ No race condition detected this run\n");
        printf("  (This doesn't mean the code is correct!)\n");
        printf("  Try running multiple times or increasing loop count\n");
    } else {
        printf("✗ Race condition detected!\n");
        int lost_updates = expected - actual;
        printf("  Lost updates: %d\n", lost_updates);
        printf("  Data race percentage: %.2f%%\n", 
               (double)lost_updates / expected * 100);
    }
    
    printf("\nExplanation:\n");
    printf("Each thread performs %d increments to the shared counter.\n", loops);
    printf("Without synchronization, some increments may be lost when\n");
    printf("threads read the same value before either writes it back.\n");
}

int main(int argc, char *argv[]) {
    /*
     * COMMAND LINE ARGUMENT PROCESSING
     * ===============================
     * 
     * The number of loops controls how likely race conditions are.
     * More loops = more opportunities for race conditions.
     */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_loops>\n", argv[0]);
        fprintf(stderr, "\nExamples:\n");
        fprintf(stderr, "  %s 1000     # Quick test (race conditions less likely)\n", argv[0]);
        fprintf(stderr, "  %s 100000   # More likely to show race conditions\n", argv[0]);
        fprintf(stderr, "  %s 1000000  # Almost guaranteed race conditions\n", argv[0]);
        fprintf(stderr, "\nPurpose:\n");
        fprintf(stderr, "  Demonstrates race conditions in multi-threaded programs.\n");
        fprintf(stderr, "  Shows why synchronization is necessary for shared data.\n");
        exit(EXIT_FAILURE);
    }
    
    loops = atoi(argv[1]);
    if (loops <= 0) {
        fprintf(stderr, "Error: Number of loops must be positive\n");
        exit(EXIT_FAILURE);
    }
    
    /*
     * PROGRAM SETUP AND INFORMATION
     * ============================
     */
    pid_t pid = getpid();
    
    printf("=============================================================\n");
    printf("Threading and Race Condition Demonstration - Process %d\n", pid);
    printf("=============================================================\n\n");
    
    printf("Configuration:\n");
    printf("  Threads:          2\n");
    printf("  Iterations/thread: %d\n", loops);
    printf("  Expected result:   %d (2 × %d)\n", loops * 2, loops);
    printf("  Shared variable:   counter (initially %d)\n", counter);
    printf("\n");
    
    /*
     * PTHREAD VARIABLES
     * ================
     * 
     * p1, p2: Thread handles for the two worker threads
     */
    pthread_t p1, p2;
    
    printf("Initial counter value: %d\n", counter);
    printf("Creating threads...\n\n");
    
    /*
     * THREAD CREATION
     * ==============
     * 
     * pthread_create() starts new threads executing the worker function.
     * 
     * Parameters:
     * - &p1: Pointer to thread handle (output)
     * - NULL: Default thread attributes
     * - worker: Function to execute in new thread
     * - (void*)1: Argument to pass to worker function (thread number)
     */
    double program_start = GetTime();
    
    int rc1 = pthread_create(&p1, NULL, worker, (void*)1);
    if (rc1 != 0) {
        fprintf(stderr, "Error creating thread 1: %d\n", rc1);
        exit(EXIT_FAILURE);
    }
    
    int rc2 = pthread_create(&p2, NULL, worker, (void*)2);
    if (rc2 != 0) {
        fprintf(stderr, "Error creating thread 2: %d\n", rc2);
        exit(EXIT_FAILURE);
    }
    
    printf("Both threads created successfully\n");
    printf("Threads are now running concurrently...\n\n");
    
    /*
     * THREAD SYNCHRONIZATION
     * =====================
     * 
     * pthread_join() waits for threads to complete before proceeding.
     * This ensures we don't read the final counter value before
     * both threads have finished modifying it.
     */
    printf("Waiting for threads to complete...\n");
    
    pthread_join(p1, NULL);  // Wait for thread 1 to finish
    printf("Thread 1 joined (completed)\n");
    
    pthread_join(p2, NULL);  // Wait for thread 2 to finish
    printf("Thread 2 joined (completed)\n");
    
    double program_end = GetTime();
    
    /*
     * RESULTS ANALYSIS
     * ===============
     * 
     * Compare expected vs actual results to detect race conditions.
     */
    printf("\n=== Final Results ===\n");
    printf("Program execution time: %.6f seconds\n", program_end - program_start);
    
    int expected_value = loops * 2;  // Each thread increments 'loops' times
    printf("Final counter value: %d\n", counter);
    printf("Expected value:      %d\n", expected_value);
    
    // Perform detailed analysis
    analyze_performance();
    analyze_race_condition(expected_value, counter);
    
    /*
     * EDUCATIONAL SUMMARY
     * ==================
     */
    printf("\n=== Key Takeaways ===\n");
    printf("1. Race conditions occur when multiple threads access shared data\n");
    printf("2. The ++ operator is NOT atomic - it involves multiple instructions\n");
    printf("3. Thread scheduling is non-deterministic - results vary between runs\n");
    printf("4. Synchronization primitives (mutexes, atomics) are necessary\n");
    printf("5. Concurrent programming requires careful design and testing\n");
    
    if (counter != expected_value) {
        printf("\n⚠️  This program demonstrates unsafe concurrent programming!\n");
        printf("   In production code, use proper synchronization mechanisms.\n");
    }
    
    return EXIT_SUCCESS;
}
