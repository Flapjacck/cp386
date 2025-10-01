#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/*
 * This program demonstrates the fork-exec pattern
 * It creates a child process that executes a different program
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
        printf("Child process before exec, PID: %d\n", getpid());
        
        // Execute a different program (ls -l)
        execl("/bin/ls", "ls", "-l", NULL);
        
        // This code will only execute if execl fails
        perror("execl failed");
        exit(1);
    } else {
        // Parent process
        printf("Parent process, PID: %d\n", getpid());
        printf("Parent created child with PID: %d\n", pid);
        
        // Wait for child to complete
        int status;
        wait(&status);
        
        printf("Child process completed with status: %d\n", WEXITSTATUS(status));
    }
    
    return 0;
}