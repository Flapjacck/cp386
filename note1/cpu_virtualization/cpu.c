/*
 * ===================================================================
 * CPU Virtualization Demonstration Program
 * ===================================================================
 * 
 * This program demonstrates one of the fundamental concepts in operating
 * systems: CPU virtualization. It shows how the OS creates the illusion
 * that each process has its own dedicated CPU, even though multiple
 * processes are actually sharing the same physical CPU through time-sharing.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. CPU Virtualization: The OS provides each process with the illusion
 *    of having its own CPU by rapidly switching between processes
 * 
 * 2. Time-Sharing: Multiple processes share CPU time through preemptive
 *    scheduling, where the OS periodically interrupts running processes
 *    and switches to other ready processes
 * 
 * 3. Context Switching: The mechanism by which the OS saves the state
 *    of one process and loads the state of another process
 * 
 * 4. Process Scheduling: The OS scheduler determines which process runs
 *    when, implementing fairness and responsiveness policies
 * 
 * REAL-WORLD ANALOGY:
 * ------------------
 * Think of a single teacher (CPU) managing multiple students (processes)
 * who all need help. The teacher rapidly moves between students, spending
 * a few seconds with each one. From each student's perspective, they have
 * the teacher's attention, but in reality, the teacher is being shared.
 * 
 * SCHEDULING ALGORITHM CONTEXT:
 * ----------------------------
 * Most modern systems use variations of Round-Robin scheduling:
 * - Each process gets a "time quantum" (typically 10-100ms)
 * - When quantum expires, process is preempted
 * - Next process in queue gets to run
 * - Preempted process goes to back of queue
 * 
 * References:
 * - OSDev Wiki: https://wiki.osdev.org/Scheduling_Algorithms
 * - OSTEP Chapter 7: CPU Scheduling
 * - Linux CFS (Completely Fair Scheduler)
 */

#include <stdio.h>      // For printf(), fprintf()
#include <stdlib.h>     // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <sys/time.h>   // For timing functions (via common.h)
#include <assert.h>     // For assertions (via common.h)
#include <unistd.h>     // For getpid() - process identification
#include "../../common.h"  // Our common utility functions

/*
 * PROGRAM BEHAVIOR ANALYSIS:
 * ========================
 * 
 * Single Instance:
 * ---------------
 * ./cpu A
 * Output: A A A A A ... (continuous, no interruption)
 * 
 * Multiple Instances:
 * ------------------
 * ./cpu A & ./cpu B & ./cpu C &
 * Output: A B C A B C A B ... (interleaved, demonstrating time-sharing)
 * 
 * The interleaving proves that:
 * 1. No process has exclusive CPU access
 * 2. OS scheduler is working (context switching)
 * 3. CPU virtualization is functioning
 * 4. Each process believes it has continuous CPU access
 */

int main(int argc, char *argv[])
{
    /*
     * Command-line argument validation
     * ===============================
     * We require exactly one argument: the string to print repeatedly.
     * This allows us to distinguish between different process instances
     * when running multiple copies simultaneously.
     */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        fprintf(stderr, "\nExamples:\n");
        fprintf(stderr, "  Single process:     %s A\n", argv[0]);
        fprintf(stderr, "  Multiple processes: %s A & %s B & %s C &\n", 
                argv[0], argv[0], argv[0]);
        fprintf(stderr, "\nPurpose:\n");
        fprintf(stderr, "  Demonstrates CPU virtualization through time-sharing.\n");
        fprintf(stderr, "  Multiple instances will show interleaved output,\n");
        fprintf(stderr, "  proving that processes share CPU time.\n");
        exit(EXIT_FAILURE);
    }
    
    char *process_id = argv[1];  // String to identify this process instance
    pid_t pid = getpid();        // Get process ID for detailed output
    
    printf("[PID %d] Starting CPU virtualization demo with identifier '%s'\n", 
           pid, process_id);
    printf("[PID %d] This process will run in an infinite loop, printing '%s'\n",
           pid, process_id);
    printf("[PID %d] OS scheduler will interleave this with other processes\n",
           pid);
    printf("[PID %d] Use Ctrl+C or 'killall cpu' to stop\n\n", pid);
    
    /*
     * INFINITE CPU-INTENSIVE LOOP
     * ==========================
     * 
     * This loop demonstrates several key OS concepts:
     * 
     * 1. CPU-Bound Process: This process continuously uses CPU without
     *    blocking on I/O operations. Such processes are good candidates
     *    for demonstrating CPU scheduling policies.
     * 
     * 2. Preemptive Scheduling: Even though this process never voluntarily
     *    yields the CPU (no system calls that block), the OS will still
     *    preempt it periodically to run other processes.
     * 
     * 3. Time Quantum: The Spin(1) call simulates work that takes about
     *    1 second. Most OS time quanta are much shorter (10-100ms), so
     *    this process will likely be preempted multiple times during
     *    each Spin() call.
     * 
     * 4. Context Switching Overhead: Each time the OS switches to/from
     *    this process, there's overhead in saving/restoring registers,
     *    memory mappings, etc. With multiple processes, you can observe
     *    this indirectly through timing variations.
     */
    unsigned long iterations = 0;  // Count how many times we've run
    double start_time = GetTime(); // Record when we started
    
    while (1) {  // Infinite loop - represents long-running process
        /*
         * CPU-Intensive Work Simulation
         * =============================
         * Spin(1) burns CPU cycles for approximately 1 second.
         * This simulates real CPU-intensive tasks like:
         * - Mathematical calculations
         * - Image/video processing
         * - Cryptographic operations
         * - Scientific simulations
         * 
         * Key Point: This is a "busy wait" - the process actively
         * consumes CPU rather than sleeping or blocking on I/O.
         */
        Spin(1);
        
        /*
         * Output with Timing Information
         * =============================
         * We print not just the process identifier, but also:
         * - Process ID (PID) for system-level identification
         * - Iteration count to see progress
         * - Elapsed time to observe scheduling effects
         * - Current timestamp for interleaving analysis
         */
        iterations++;
        double current_time = GetTime();
        double elapsed = current_time - start_time;
        
        printf("[PID %5d][Iter %6lu][Time %8.2fs] %s\n", 
               pid, iterations, elapsed, process_id);
        
        /*
         * Optional: Flush output buffer
         * ============================
         * This ensures output appears immediately, which is important
         * for observing the real-time interleaving of multiple processes.
         * Without this, output might be buffered and appear in chunks.
         */
        fflush(stdout);
    }
    
    /*
     * This point is never reached due to the infinite loop above.
     * In a real application, you would have proper cleanup code here:
     * - Close file descriptors
     * - Free allocated memory
     * - Perform graceful shutdown
     */
    return EXIT_SUCCESS;
}
