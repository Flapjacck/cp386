#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Minimal execvp() example
void simple_exec_example() {
    char *argv[3];
    argv[0] = "echo";
    argv[1] = "hello";
    argv[2] = NULL;
    
    printf("About to execute: echo hello\n");
    execvp("echo", argv);
    printf("exec error\n"); // This should not print if exec succeeds
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv; // Suppress unused parameter warnings
    int rc = fork();
    
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // Child process
        simple_exec_example();
    } else {
        // Parent process
        wait(NULL);
        printf("Child completed execution\n");
    }
    
    return 0;
}
