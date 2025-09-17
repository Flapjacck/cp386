/*
 * ===================================================================
 * CP386 Operating Systems Course - Common Utilities Header
 * ===================================================================
 * 
 * This header provides common utility functions used throughout the
 * course demonstrations. These functions help simulate real-world
 * operating system behaviors and timing mechanisms.
 * 
 * Key Components:
 * - High-precision timing functions for performance measurement
 * - CPU burning functions for demonstrating scheduling
 * - Thread synchronization utilities
 * 
 * References:
 * - OSTEP (Operating Systems: Three Easy Pieces) by Remzi H. Arpaci-Dusseau
 * - Linux kernel timing mechanisms
 * - POSIX threading standards
 */

#ifndef __common_h__
#define __common_h__

#include <sys/time.h>    // For gettimeofday() - microsecond precision timing
#include <sys/stat.h>    // For file status operations
#include <assert.h>      // For runtime assertion checking
#include <pthread.h>     // For POSIX threading support

/*
 * GetTime() - High-Precision Timer Function
 * =========================================
 * 
 * Purpose: Returns the current time in seconds with microsecond precision.
 * This function is crucial for measuring execution times, scheduling
 * quantum measurements, and performance analysis in operating systems.
 * 
 * Implementation Details:
 * - Uses gettimeofday() system call for microsecond precision
 * - Returns time as double for easy arithmetic operations
 * - Combines seconds and microseconds into single floating-point value
 * 
 * Usage in OS Context:
 * - Process scheduling quantum measurement
 * - System call latency analysis
 * - Context switching overhead calculation
 * - Performance benchmarking
 * 
 * Return: Current time in seconds since Unix epoch (Jan 1, 1970)
 *         with microsecond precision (e.g., 1609459200.123456)
 */
double GetTime() {
    struct timeval t;
    
    // gettimeofday() fills timeval structure with current time
    // t.tv_sec:  seconds since Unix epoch
    // t.tv_usec: microseconds within current second (0-999999)
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);  // Ensure system call succeeded
    
    // Convert to double: seconds + (microseconds / 1,000,000)
    // This gives us second.microsecond precision
    return (double) t.tv_sec + (double) t.tv_usec/1e6;
}

/*
 * Spin() - CPU Burn Function for Scheduling Demonstrations
 * ========================================================
 * 
 * Purpose: Consumes CPU cycles for a specified duration without yielding
 * the processor. This simulates CPU-intensive work and demonstrates
 * time-sharing, preemptive scheduling, and resource competition.
 * 
 * Operating System Concepts Demonstrated:
 * - Time-sharing: Multiple processes competing for CPU time
 * - Preemptive scheduling: OS interrupts running processes
 * - CPU virtualization: Illusion of dedicated CPU per process
 * - Context switching: OS switches between processes
 * - Scheduling quantum: Time slice allocated to each process
 * 
 * Real-World Analogies:
 * - Mathematical calculations (prime number generation)
 * - Cryptographic operations (encryption/decryption)
 * - Image/video processing algorithms
 * - Scientific simulations
 * 
 * Implementation Notes:
 * - Busy-wait loop (actively consumes CPU)
 * - No system calls that might block or yield
 * - Designed to be interruptible by scheduler
 * - Duration in seconds (floating-point for precision)
 * 
 * Parameters:
 * @howlong: Duration in seconds to burn CPU cycles
 */
void Spin(int howlong) {
    double start_time = GetTime();
    
    // Busy-wait loop: continuously check elapsed time
    // This simulates CPU-intensive work that doesn't block
    // The OS scheduler can preempt this process during execution
    while ((GetTime() - start_time) < (double) howlong) {
        // Intentionally empty - just burning CPU cycles
        // In real applications, this would be:
        // - Complex calculations
        // - Data processing
        // - Algorithm execution
        ; 
    }
}

/*
 * Additional Timing Utilities
 * ===========================
 * These macros provide convenient ways to measure code execution time
 * and demonstrate performance characteristics of different operations.
 */

// Macro to measure execution time of a code block
#define TIME_BLOCK(description, block) do { \
    double start = GetTime(); \
    block; \
    double end = GetTime(); \
    printf("[TIMING] %s: %.6f seconds\n", description, end - start); \
} while(0)

// Convert nanoseconds to seconds (useful for high-precision timing)
#define NS_TO_SEC(ns) ((double)(ns) / 1000000000.0)

// Convert microseconds to seconds
#define US_TO_SEC(us) ((double)(us) / 1000000.0)

// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec) * 1000.0)

#endif // __common_h__
