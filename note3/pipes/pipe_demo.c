/*
 * pipe_demo.c - Inter-Process Communication with Pipes
 * 
 * Educational Purpose:
 * This program demonstrates **Unix pipes** - one of the most fundamental
 * mechanisms for inter-process communication (IPC). Pipes allow one process
 * to send data directly to another process, enabling the composition of
 * complex operations from simple programs.
 *
 * Core OS Concepts Demonstrated:
 * 1. Unnamed pipes (anonymous pipes) for parent-child communication
 * 2. File descriptor duplication and redirection
 * 3. Producer-consumer pattern in concurrent systems
 * 4. How shell pipe operator (|) works internally
 * 5. Process synchronization through pipe EOF signaling
 * 6. Buffered I/O and blocking/non-blocking semantics
 *
 * Real-world Shell Command Simulated:
 * echo -e "hello world\nthis is a test\nof the pipe system" | wc
 *
 * Pipe Theory:
 * ============
 * A pipe is a kernel buffer that connects the output of one process
 * to the input of another. Data written to the write end appears at
 * the read end, implementing a FIFO (First-In-First-Out) queue.
 *
 * References:
 * - OSTEP Chapter 36: I/O Redirection  
 * - Stevens & Rago: Advanced Programming in the UNIX Environment
 * - OSDev Wiki: https://wiki.osdev.org/Pipes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    /*
     * STEP 1: Create Pipe
     * ==================
     * pipe() creates a communication channel with two file descriptors:
     * - p[0]: Read end of the pipe (data comes out here)
     * - p[1]: Write end of the pipe (data goes in here)
     * 
     * Pipe Buffer Visualization:
     * 
     * p[1] (write end)  →  [Kernel Buffer]  →  p[0] (read end)
     *     Producer                FIFO                Consumer
     *     Process                Queue               Process
     * 
     * Key Properties:
     * - Data flows unidirectionally (write end → read end)
     * - Kernel manages the buffer (typically 4KB or 64KB)
     * - Reading from empty pipe blocks until data arrives
     * - Writing to full pipe blocks until space is available
     * - When all write ends close, read returns EOF (0 bytes)
     */
    int p[2];  // Array to hold pipe file descriptors
    char *argv_wc[2];
    
    // Set up arguments for wc command
    argv_wc[0] = "wc";    // Command name
    argv_wc[1] = NULL;    // NULL-terminated argument array
    
    /*
     * Create the pipe before forking so both processes inherit it.
     * After fork(), both parent and child have copies of both FDs.
     */
    if (pipe(p) < 0) {
        fprintf(stderr, "pipe failed\n");
        exit(1);
    }
    
    printf("Created pipe: read_fd=%d, write_fd=%d\n", p[0], p[1]);
    printf("Simulating: echo 'data' | wc\n");
    printf("Parent will write text, child will count it with wc.\n\n");
    
    /*
     * STEP 2: Fork Child Process
     * ==========================
     * After fork(), we have:
     * 
     * Parent Process:          Child Process:
     * ┌─────────────────┐     ┌─────────────────┐
     * │ FD Table:       │     │ FD Table:       │
     * │ 0: stdin        │     │ 0: stdin        │
     * │ 1: stdout       │     │ 1: stdout       │  
     * │ 2: stderr       │     │ 2: stderr       │
     * │ p[0]: pipe read │     │ p[0]: pipe read │ ← Both inherit
     * │ p[1]: pipe write│     │ p[1]: pipe write│ ← same pipe FDs
     * └─────────────────┘     └─────────────────┘
     */
    if (fork() == 0) {
        /*
         * CHILD PROCESS: Set up as pipe consumer (reader)
         * ==============================================
         * The child will run the 'wc' command, which reads from stdin.
         * We need to redirect stdin to read from our pipe.
         */
        
        printf("[CHILD] Setting up to receive data from pipe...\n");
        
        /*
         * Redirect stdin to pipe read end:
         * 1. Close current stdin (FD 0)
         * 2. Duplicate pipe read end to FD 0
         * 3. Close original pipe FDs (cleanup)
         */
        close(0);       // Close stdin (FD 0)
        dup(p[0]);      // Duplicate p[0] to FD 0 (stdin now reads from pipe)
        close(p[0]);    // Close original pipe read FD (we have copy at FD 0)
        close(p[1]);    // Close pipe write end (child doesn't write)
        
        /*
         * File Descriptor Table after redirection:
         * ┌─────────────────────────┐
         * │ 0: pipe read (was stdin)│ ← stdin now reads from pipe
         * │ 1: stdout (terminal)    │ ← wc output goes to terminal
         * │ 2: stderr (terminal)    │
         * └─────────────────────────┘
         */
        
        printf("[CHILD] Executing wc command...\n");
        
        /*
         * Execute wc command.
         * wc will read from stdin (which is now our pipe) and count:
         * - Lines (-l)
         * - Words (-w) 
         * - Characters (-c)
         * Output goes to stdout (terminal).
         */
        execvp("wc", argv_wc);
        
        // If we reach here, exec failed
        fprintf(stderr, "exec failed\n");
        exit(1);
        
    } else {
        /*
         * PARENT PROCESS: Set up as pipe producer (writer)
         * ===============================================
         * The parent will write data to the pipe, which the child
         * will read and process with wc.
         */
        
        printf("[PARENT] Sending data through pipe...\n");
        
        /*
         * Close read end - parent only writes to pipe.
         * This is important for proper EOF handling:
         * - Child will get EOF when ALL write ends are closed
         * - If parent keeps read end open, child might hang
         */
        close(p[0]);    // Close read end (parent doesn't read)
        
        /*
         * Write test data to pipe.
         * Each write() call sends data to the kernel pipe buffer.
         * The child's wc command will receive this data as if
         * it came from a file or keyboard input.
         */
        printf("[PARENT] Writing: 'hello world'\n");
        write(p[1], "hello world\n", 12);
        
        printf("[PARENT] Writing: 'this is a test'\n");
        write(p[1], "this is a test\n", 15);
        
        printf("[PARENT] Writing: 'of the pipe system'\n");
        write(p[1], "of the pipe system\n", 19);
        
        /*
         * CRITICAL: Close write end to signal EOF
         * =======================================
         * When we close the write end, the kernel marks the pipe as
         * "no more writers". The next time the child tries to read,
         * it will get EOF (0 bytes), causing wc to finish and output results.
         * 
         * Without this close(), the child would block forever waiting
         * for more input that will never come.
         */
        printf("[PARENT] Closing pipe (sending EOF signal)...\n");
        close(p[1]);    // Signal EOF to child
        
        /*
         * Wait for child process to complete.
         * The child (wc) will process all the data we sent,
         * count lines/words/characters, and output the results.
         */
        printf("[PARENT] Waiting for child to process data...\n");
        wait(NULL);     // Block until child terminates
        
        printf("[PARENT] Pipe communication completed!\n");
    }
    
    /*
     * EDUCATIONAL SUMMARY:
     * ===================
     * This program demonstrates the fundamental mechanism behind
     * shell pipes (|). When you type "cmd1 | cmd2" in a shell:
     * 
     * 1. Shell creates a pipe
     * 2. Shell forks two children  
     * 3. First child: redirects stdout to pipe write end, exec(cmd1)
     * 4. Second child: redirects stdin to pipe read end, exec(cmd2)
     * 5. Shell waits for both children to complete
     * 
     * Key Insights:
     * - Pipes enable data flow between independent programs
     * - File descriptor manipulation is key to I/O redirection
     * - EOF signaling (closing write end) is crucial for termination
     * - This mechanism scales to arbitrary pipeline lengths
     * 
     * Try these experiments:
     * 1. Compare with: echo -e "hello world\nthis is a test\nof the pipe system" | wc
     * 2. Trace system calls: strace -e pipe,fork,dup,exec ./pipe_demo
     * 3. Modify to use different commands (sort, grep, etc.)
     */
    return 0;
}
