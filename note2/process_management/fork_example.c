#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/*
 * This program demonstrates the basic process creation using fork()
 * It creates a child process and shows both parent and child execution paths
 */

int main() {
    pid_t pid;
    
    // Fork a child process
    pid = fork();
    
    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process executing\n");
        printf("Child PID: %d\n", getpid());
        printf("Child's Parent PID: %d\n", getppid());
    } else {
        // Parent process
        printf("Parent process executing\n");
        printf("Parent PID: %d\n", getpid());
        printf("Child PID: %d\n", pid);
        
        // Wait for child to finish to prevent zombie process
        wait(NULL);
    }
    
    printf("Process %d exiting\n", getpid());
    return 0;
}