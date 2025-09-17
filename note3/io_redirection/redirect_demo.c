/*
 * redirect_demo.c - Comprehensive I/O Redirection Demonstrations
 * 
 * Educational Purpose:
 * This program provides multiple examples of I/O redirection techniques,
 * demonstrating both output and input redirection. It shows how file
 * descriptors work at the system level and how shells implement redirection.
 *
 * Core OS Concepts Demonstrated:
 * 1. File descriptor manipulation and inheritance
 * 2. The relationship between file descriptors and stdio
 * 3. Input redirection (< operator simulation)
 * 4. Output redirection (> operator simulation)  
 * 5. Process communication through file descriptor manipulation
 * 6. The Unix "everything is a file" philosophy
 *
 * Real-world Applications:
 * - Shell I/O redirection (bash, zsh, etc.)
 * - Log file processing
 * - Batch job input/output handling
 * - System administration scripting
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

/*
 * fd_demo() - File Descriptor Inheritance Demonstration
 * ====================================================
 * 
 * This function demonstrates that file descriptors are inherited
 * by child processes created with fork(). Both parent and child
 * can write to the same file descriptor (stdout in this case).
 * 
 * Key Learning Points:
 * - fork() duplicates the entire file descriptor table
 * - Changes to file descriptor contents affect both processes
 * - File descriptor numbers remain the same in both processes
 * - This inheritance is fundamental to I/O redirection
 */
void fd_demo() {
    printf("File descriptor demo:\n");
    printf("Demonstrating FD inheritance across fork()...\n");
    
    /*
     * File Descriptor Table Before Fork:
     * FD 0: stdin  (keyboard)
     * FD 1: stdout (terminal) ← Both processes will share this
     * FD 2: stderr (terminal)
     */
    
    if (fork() == 0) {
        /*
         * CHILD PROCESS:
         * Write directly to file descriptor 1 (stdout) using write() syscall.
         * This bypasses stdio buffering and goes directly to the kernel.
         * The write() call is atomic for small writes.
         */
        write(1, "hello ", 6);  // FD 1 = stdout, write 6 bytes
        exit(0);  // Child terminates immediately
    } else {
        /*
         * PARENT PROCESS:
         * Wait for child to complete, then write to the same FD.
         * This demonstrates that both processes share the same
         * underlying file description (terminal output).
         */
        wait(NULL);  // Synchronize with child completion
        write(1, "world\n", 6);  // Complete the message
    }
    
    /*
     * Expected output: "hello world"
     * This shows that both processes successfully wrote to the same
     * file descriptor, demonstrating FD inheritance.
     */
}

/*
 * input_redirect_demo() - Input Redirection Implementation
 * =======================================================
 * 
 * This function demonstrates input redirection - making a program
 * read from a file instead of the keyboard. This simulates the
 * shell's "command < file" functionality.
 * 
 * Steps Demonstrated:
 * 1. Create input file with test data
 * 2. Fork child process
 * 3. In child: redirect stdin to file
 * 4. In child: exec 'cat' command
 * 5. cat reads from file instead of keyboard
 */
void input_redirect_demo() {
    printf("\nInput redirection demo:\n");
    printf("Simulating: cat < input.txt\n");
    
    /*
     * STEP 1: Create Test Input File
     * =============================
     * We need a file with content for the redirection demo.
     * This simulates having an existing file to redirect from.
     */
    int fd = open("input.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if (fd >= 0) {
        /*
         * Write test content to the file.
         * This will be the "input" that cat reads from stdin.
         */
        const char *content = "Hello from input file!\nLine 2\nLine 3\n";
        write(fd, content, strlen(content));
        close(fd);  // Close file after writing
        printf("Created input.txt with test content.\n");
    } else {
        perror("Failed to create input.txt");
        return;
    }
    
    /*
     * STEP 2: Prepare Command Arguments
     * =================================
     * Set up argument array for execvp().
     * cat with no arguments reads from stdin and writes to stdout.
     */
    char *argv[2];
    argv[0] = "cat";    // Command name
    argv[1] = NULL;     // NULL-terminated array
    
    /*
     * STEP 3: Fork and Redirect
     * =========================
     */
    if (fork() == 0) {
        /*
         * CHILD PROCESS: Implement Input Redirection
         * ==========================================
         * This is exactly how "cat < input.txt" works in the shell.
         */
        
        /*
         * Close stdin (FD 0) to free up the file descriptor.
         * 
         * File Descriptor Table after close(0):
         * FD 0: [AVAILABLE] ← stdin closed, ready for reuse
         * FD 1: stdout (terminal)
         * FD 2: stderr (terminal)
         */
        close(0);  // Close standard input
        
        /*
         * Open input file - it will get assigned FD 0 (stdin).
         * Now when cat tries to read from stdin, it reads from our file!
         * 
         * File Descriptor Table after open():
         * FD 0: input.txt ← stdin now redirected to file!
         * FD 1: stdout (terminal)
         * FD 2: stderr (terminal)
         */
        int input_fd = open("input.txt", O_RDONLY);
        if (input_fd < 0) {
            perror("Failed to open input.txt");
            exit(1);
        }
        
        /*
         * Execute cat command.
         * cat reads from stdin (now our file) and writes to stdout (terminal).
         * This demonstrates input redirection without cat knowing about it!
         */
        execvp("cat", argv);
        
        // If we reach here, exec() failed
        perror("exec cat failed");
        exit(1);
        
    } else {
        /*
         * PARENT PROCESS: Wait for Child
         * ==============================
         * Parent waits for the redirection demo to complete.
         */
        wait(NULL);
        printf("Input redirection completed.\n");
    }
    
    /*
     * EDUCATIONAL NOTES:
     * =================
     * 1. The cat command never knows it's reading from a file instead of keyboard
     * 2. This is the power of file descriptors - programs are I/O agnostic
     * 3. The shell implements "cat < input.txt" using this exact mechanism
     * 4. Any program can be made to read from files instead of keyboard
     * 5. This works because Unix treats files, terminals, pipes all the same way
     */
}

/*
 * MAIN FUNCTION: Orchestrate Demonstrations
 * =========================================
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    printf("=== I/O Redirection Comprehensive Demo ===\n\n");
    
    /*
     * Run both demonstrations to show different aspects
     * of file descriptor manipulation and I/O redirection.
     */
    fd_demo();              // Show FD inheritance
    input_redirect_demo();  // Show input redirection
    
    printf("\n=== Demo Summary ===\n");
    printf("1. File descriptors are inherited by child processes\n");
    printf("2. close() + open() sequence redirects I/O streams\n");
    printf("3. Programs are unaware of redirection - they just use stdin/stdout\n");
    printf("4. This is how shells implement < and > operators\n");
    printf("\nFiles created: input.txt\n");
    printf("Experiment: Try 'cat input.txt' to see the test content.\n");
    
    return 0;
}
