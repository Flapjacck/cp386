#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

/*
 * This program demonstrates process scheduling concepts
 * It creates CPU-bound and I/O-bound processes and shows their behavior
 */

// Simulates a CPU-bound process that does computation
void cpu_bound_process(int process_id, int iterations) {
    printf("CPU-bound process %d started (PID: %d)\n", process_id, getpid());
    
    clock_t start_time = clock();
    
    // Perform CPU-intensive computation
    volatile double result = 0;
    for (int i = 0; i < iterations; i++) {
        result += i / 2.0;
        result *= 1.1;
        
        // Occasionally report progress
        if (i % (iterations / 10) == 0) {
            printf("CPU-bound process %d: %d%% complete\n", 
                   process_id, (i * 100) / iterations);
        }
    }
    
    clock_t end_time = clock();
    double cpu_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("CPU-bound process %d completed in %.2f seconds\n", 
           process_id, cpu_time);
    exit(0);
}

// Simulates an I/O-bound process with frequent I/O operations
void io_bound_process(int process_id, int operations) {
    printf("I/O-bound process %d started (PID: %d)\n", process_id, getpid());
    
    time_t start_time = time(NULL);
    
    for (int i = 0; i < operations; i++) {
        printf("I/O-bound process %d: performing I/O operation %d\n", 
               process_id, i + 1);
        
        // Simulate I/O operation by sleeping
        usleep(200000);  // 200ms
        
        // Small amount of computation between I/O
        volatile int calc = 0;
        for (int j = 0; j < 10000; j++) {
            calc += j;
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    printf("I/O-bound process %d completed in %.2f seconds\n", 
           process_id, elapsed);
    exit(0);
}

int main() {
    printf("Process Scheduling Demonstration\n");
    printf("Parent PID: %d\n\n", getpid());
    
    // Create two CPU-bound processes
    pid_t cpu_pid1 = fork();
    if (cpu_pid1 == 0) {
        cpu_bound_process(1, 100000000);  // 100 million iterations
    }
    
    pid_t cpu_pid2 = fork();
    if (cpu_pid2 == 0) {
        cpu_bound_process(2, 100000000);  // 100 million iterations
    }
    
    // Create two I/O-bound processes
    pid_t io_pid1 = fork();
    if (io_pid1 == 0) {
        io_bound_process(1, 20);  // 20 I/O operations
    }
    
    pid_t io_pid2 = fork();
    if (io_pid2 == 0) {
        io_bound_process(2, 20);  // 20 I/O operations
    }
    
    // Parent waits for all children to complete
    printf("Parent waiting for all child processes to complete\n");
    
    for (int i = 0; i < 4; i++) {
        int status;
        pid_t terminated_pid = wait(&status);
        printf("Child process (PID: %d) terminated with status %d\n", 
               terminated_pid, WEXITSTATUS(status));
    }
    
    printf("\nObservation: Notice how I/O-bound processes finish faster in wall-clock time\n");
    printf("despite their frequent blocking, while CPU-bound processes consume more CPU time.\n");
    printf("This demonstrates why schedulers prioritize I/O-bound processes to maintain\n");
    printf("system responsiveness and maximize CPU utilization.\n");
    
    return 0;
}