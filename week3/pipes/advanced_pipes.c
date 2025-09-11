#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

// Simple pipe demonstration - equivalent to: echo "Hello World" | wc
void simple_pipe_demo() {
    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    if (fork() == 0) {
        // Child: will run wc
        close(pipefd[1]);          // Close write end
        dup2(pipefd[0], 0);        // Redirect stdin to read end of pipe
        close(pipefd[0]);          // Close original read end
        
        execlp("wc", "wc", NULL);
        perror("execlp");
        exit(1);
    } else {
        // Parent: will write data
        close(pipefd[0]);          // Close read end
        
        char *message = "Hello World\nFrom the pipe\nThird line\n";
        write(pipefd[1], message, strlen(message));
        close(pipefd[1]);          // Close write end (EOF signal)
        
        wait(NULL);                // Wait for child
    }
}

// Bidirectional communication using two pipes
void bidirectional_pipe_demo() {
    int pipe1[2], pipe2[2];  // pipe1: parent->child, pipe2: child->parent
    
    printf("\nBidirectional pipe demo:\n");
    
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }
    
    if (fork() == 0) {
        // Child process
        close(pipe1[1]);  // Close write end of pipe1
        close(pipe2[0]);  // Close read end of pipe2
        
        char buffer[100];
        read(pipe1[0], buffer, sizeof(buffer));
        printf("Child received: %s", buffer);
        
        char response[] = "Hello from child!\n";
        write(pipe2[1], response, strlen(response));
        
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    } else {
        // Parent process
        close(pipe1[0]);  // Close read end of pipe1
        close(pipe2[1]);  // Close write end of pipe2
        
        char message[] = "Hello from parent!\n";
        write(pipe1[1], message, strlen(message));
        
        char buffer[100];
        read(pipe2[0], buffer, sizeof(buffer));
        printf("Parent received: %s", buffer);
        
        close(pipe1[1]);
        close(pipe2[0]);
        wait(NULL);
    }
}

int main() {
    printf("Simple pipe demo (echo | wc):\n");
    simple_pipe_demo();
    
    bidirectional_pipe_demo();
    
    return 0;
}
