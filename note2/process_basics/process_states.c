#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

/*
 * This program demonstrates process states and transitions
 * It creates multiple child processes that transition through different states
 */

int main() {
    printf("Process States Demonstration\n");
    printf("Parent PID: %d\n\n", getpid());
    
    // Create first child process
    pid_t pid1 = fork();
    
    if (pid1 < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid1 == 0) {
        // First child process (CPU-bound)
        printf("Child 1 (PID: %d) created - CPU-bound process\n", getpid());
        printf("Child 1 entering RUNNING state\n");
        
        // CPU-intensive work
        volatile unsigned long i;
        for (i = 0; i < 1000000000; i++) {
            // Just burn CPU cycles
        }
        
        printf("Child 1 completed CPU work, entering TERMINATED state\n");
        exit(0);
    }
    
    // Create second child process in parent
    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid2 == 0) {
        // Second child process (I/O-bound)
        printf("Child 2 (PID: %d) created - I/O-bound process\n", getpid());
        printf("Child 2 entering WAITING state for I/O\n");
        
        // Simulate I/O wait
        sleep(3);
        
        printf("Child 2 finished I/O, back to READY/RUNNING state\n");
        printf("Child 2 entering TERMINATED state\n");
        exit(0);
    }
    
    // Parent process continues
    printf("Parent has created two children (PIDs: %d, %d)\n", pid1, pid2);
    printf("Parent is in RUNNING state\n");
    
    // Wait for first child
    int status1;
    printf("Parent waiting for Child 1 to terminate\n");
    waitpid(pid1, &status1, 0);
    printf("Child 1 (PID: %d) terminated with status: %d\n", 
           pid1, WEXITSTATUS(status1));
    
    // Wait for second child
    int status2;
    printf("Parent waiting for Child 2 to terminate\n");
    waitpid(pid2, &status2, 0);
    printf("Child 2 (PID: %d) terminated with status: %d\n", 
           pid2, WEXITSTATUS(status2));
    
    printf("Parent process entering TERMINATED state\n");
    return 0;
}