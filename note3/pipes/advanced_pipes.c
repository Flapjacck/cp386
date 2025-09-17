/*
 * advanced_pipes.c - Advanced Inter-Process Communication Patterns
 * 
 * Educational Purpose:
 * This program demonstrates advanced pipe usage patterns including
 * bidirectional communication and proper pipe management. It shows
 * how to build complex IPC systems using multiple pipes and proper
 * file descriptor management.
 *
 * Core OS Concepts Demonstrated:
 * 1. Simple unidirectional pipes (producer → consumer)
 * 2. Bidirectional communication using two pipes
 * 3. Proper file descriptor management and cleanup
 * 4. dup2() vs dup() for precise FD control
 * 5. Process synchronization through pipe communications
 * 6. Advanced IPC patterns used in system programming
 *
 * Real-world Applications:
 * - Client-server communication
 * - Parent-child process coordination
 * - Shell pipeline implementation
 * - Network proxy/gateway systems
 * - Database query processing pipelines
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/*
 * simple_pipe_demo() - Unidirectional Pipe Communication
 * =====================================================
 * 
 * Demonstrates the standard pipe pattern used by shells for
 * commands like "echo 'data' | wc". This is the foundation
 * of Unix pipeline processing.
 * 
 * Pipe Data Flow:
 * Parent (echo) → [Pipe Buffer] → Child (wc)
 */
void simple_pipe_demo() {
    printf("=== Simple Pipe Demo (echo | wc) ===\n");
    
    /*
     * Create pipe before forking.
     * pipefd[0] = read end, pipefd[1] = write end
     * 
     * Pipe Visualization:
     * 
     * Write End                Read End
     * pipefd[1] ────→ [Buffer] ────→ pipefd[0]
     *   Parent                        Child
     *  (producer)                  (consumer)
     */
    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    printf("Created pipe: read_fd=%d, write_fd=%d\n", pipefd[0], pipefd[1]);
    
    /*
     * Fork creates two processes, both inheriting the pipe FDs.
     * We'll set up a classic producer-consumer relationship.
     */
    if (fork() == 0) {
        /*
         * CHILD PROCESS: Consumer (wc command)
         * ===================================
         * Child will run 'wc' and read from the pipe instead of stdin.
         */
        
        printf("[CHILD] Setting up as pipe consumer (wc)...\n");
        
        /*
         * Close write end - child only reads.
         * This is crucial for EOF detection: when all write ends
         * are closed, reads will return 0 (EOF).
         */
        close(pipefd[1]);          // Close write end
        
        /*
         * Redirect stdin to pipe using dup2().
         * dup2(oldfd, newfd) makes newfd a copy of oldfd.
         * After this, stdin (FD 0) reads from our pipe.
         */
        dup2(pipefd[0], 0);        // stdin now reads from pipe
        close(pipefd[0]);          // Close original read end (we have copy at FD 0)
        
        /*
         * File Descriptor State:
         * FD 0: pipe read end (stdin redirected)
         * FD 1: stdout (terminal)
         * FD 2: stderr (terminal)
         */
        
        printf("[CHILD] Executing wc to count pipe data...\n");
        
        /*
         * Execute word count program.
         * wc reads from stdin (our pipe) and outputs to stdout (terminal).
         * This exactly simulates "echo 'data' | wc" behavior.
         */
        execlp("wc", "wc", NULL);
        perror("execlp");
        exit(1);
        
    } else {
        /*
         * PARENT PROCESS: Producer (echo command simulation)
         * =================================================
         * Parent writes data to the pipe, simulating echo output.
         */
        
        printf("[PARENT] Setting up as pipe producer (echo)...\n");
        
        /*
         * Close read end - parent only writes.
         * This ensures proper EOF behavior when parent finishes.
         */
        close(pipefd[0]);          // Close read end
        
        /*
         * Send test data through pipe.
         * This data will appear as stdin to the child's wc command.
         */
        char *message = "Hello World\nFrom the pipe\nThird line\n";
        printf("[PARENT] Sending message (%zu bytes):\n%s", strlen(message), message);
        
        ssize_t bytes_written = write(pipefd[1], message, strlen(message));
        printf("[PARENT] Wrote %zd bytes to pipe\n", bytes_written);
        
        /*
         * CRITICAL: Close write end to signal EOF.
         * When all write ends close, child's read() returns 0,
         * causing wc to finish processing and display results.
         */
        close(pipefd[1]);          // Signal EOF to child
        
        printf("[PARENT] Closed pipe, waiting for child...\n");
        wait(NULL);                // Wait for child to complete
        printf("[PARENT] Child completed. Pipe demo finished.\n\n");
    }
}

/*
 * bidirectional_pipe_demo() - Two-Way Communication
 * ================================================
 * 
 * Demonstrates bidirectional communication between parent and child
 * using two pipes. This pattern is used in client-server systems,
 * remote procedure calls, and interactive shell implementations.
 * 
 * Communication Pattern:
 * 
 * Parent Process              Child Process
 * ┌─────────────┐            ┌─────────────┐
 * │             │   pipe1    │             │
 * │   write  ───┼────────→───┼──→ read     │
 * │             │            │             │
 * │   read   ←──┼────────←───┼───  write   │
 * │             │   pipe2    │             │
 * └─────────────┘            └─────────────┘
 */
void bidirectional_pipe_demo() {
    /*
     * Create two pipes for bidirectional communication:
     * pipe1: parent writes → child reads
     * pipe2: child writes → parent reads
     */
    int pipe1[2], pipe2[2];  // Two separate pipes
    
    printf("=== Bidirectional Pipe Demo ===\n");
    printf("Setting up two-way communication...\n");
    
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }
    
    printf("Created pipe1 (parent→child): read=%d, write=%d\n", pipe1[0], pipe1[1]);
    printf("Created pipe2 (child→parent): read=%d, write=%d\n", pipe2[0], pipe2[1]);
    
    if (fork() == 0) {
        /*
         * CHILD PROCESS: Bidirectional Communication Endpoint
         * ==================================================
         * Child reads from pipe1 and writes to pipe2.
         */
        
        printf("[CHILD] Setting up bidirectional communication...\n");
        
        /*
         * Close unused pipe ends:
         * - Child reads from pipe1, so close pipe1 write end
         * - Child writes to pipe2, so close pipe2 read end
         * 
         * Child's Pipe Usage:
         * pipe1[0]: read from parent
         * pipe2[1]: write to parent
         */
        close(pipe1[1]);  // Close write end of pipe1 (unused by child)
        close(pipe2[0]);  // Close read end of pipe2 (unused by child)
        
        /*
         * Receive message from parent.
         * This demonstrates the parent→child communication path.
         */
        char buffer[100];
        printf("[CHILD] Waiting for message from parent...\n");
        ssize_t bytes_read = read(pipe1[0], buffer, sizeof(buffer) - 1);
        buffer[bytes_read] = '\0';  // Null-terminate string
        
        printf("[CHILD] Received from parent: %s", buffer);
        
        /*
         * Send response back to parent.
         * This demonstrates the child→parent communication path.
         */
        char response[] = "Hello from child process!\n";
        printf("[CHILD] Sending response to parent...\n");
        write(pipe2[1], response, strlen(response));
        
        /*
         * Clean up child's pipe ends.
         * Closing write end signals EOF to parent.
         */
        close(pipe1[0]);
        close(pipe2[1]);
        
        printf("[CHILD] Communication complete, exiting.\n");
        exit(0);
        
    } else {
        /*
         * PARENT PROCESS: Bidirectional Communication Endpoint
         * ===================================================
         * Parent writes to pipe1 and reads from pipe2.
         */
        
        printf("[PARENT] Setting up bidirectional communication...\n");
        
        /*
         * Close unused pipe ends:
         * - Parent writes to pipe1, so close pipe1 read end
         * - Parent reads from pipe2, so close pipe2 write end
         * 
         * Parent's Pipe Usage:
         * pipe1[1]: write to child
         * pipe2[0]: read from child
         */
        close(pipe1[0]);  // Close read end of pipe1 (unused by parent)
        close(pipe2[1]);  // Close write end of pipe2 (unused by parent)
        
        /*
         * Send initial message to child.
         * This starts the bidirectional conversation.
         */
        char message[] = "Hello from parent process!\n";
        printf("[PARENT] Sending message to child...\n");
        write(pipe1[1], message, strlen(message));
        
        /*
         * Wait for child's response.
         * This completes the round-trip communication.
         */
        char buffer[100];
        printf("[PARENT] Waiting for response from child...\n");
        ssize_t bytes_read = read(pipe2[0], buffer, sizeof(buffer) - 1);
        buffer[bytes_read] = '\0';  // Null-terminate string
        
        printf("[PARENT] Received from child: %s", buffer);
        
        /*
         * Clean up parent's pipe ends and wait for child.
         */
        close(pipe1[1]);
        close(pipe2[0]);
        
        printf("[PARENT] Waiting for child to exit...\n");
        wait(NULL);
        printf("[PARENT] Bidirectional communication completed!\n\n");
    }
}

/*
 * MAIN FUNCTION: Demonstrate Pipe Patterns
 * ========================================
 */
int main() {
    printf("Advanced Pipe Communication Demonstrations\n");
    printf("==========================================\n\n");
    
    /*
     * Run demonstrations in sequence to show different
     * pipe communication patterns and techniques.
     */
    
    // Demo 1: Simple unidirectional pipe (like shell |)
    simple_pipe_demo();
    
    // Demo 2: Bidirectional communication using two pipes
    bidirectional_pipe_demo();
    
    printf("=== Summary of Pipe Concepts ===\n");
    printf("1. Unidirectional pipes: Producer → Consumer (shell pipes)\n");
    printf("2. Bidirectional pipes: Client ⟷ Server (two separate pipes)\n");
    printf("3. File descriptor management: Close unused ends for proper EOF\n");
    printf("4. Process synchronization: Pipes naturally coordinate processes\n");
    printf("5. Data buffering: Kernel manages pipe buffer (typically 64KB)\n");
    printf("\nThese patterns form the basis of complex IPC systems!\n");
    
    return 0;
}
