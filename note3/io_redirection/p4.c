/*
 * p4.c - Output Redirection Demonstration
 * 
 * Educational Purpose:
 * This program demonstrates the fundamental concept of output redirection,
 * which is a cornerstone of Unix-like operating systems. By manipulating
 * file descriptors, we can redirect the output of any command to a file
 * instead of the terminal.
 *
 * Core OS Concepts Demonstrated:
 * 1. File descriptor manipulation (close + open sequence)
 * 2. The Unix philosophy: "everything is a file"
 * 3. How shells implement output redirection (>)
 * 4. Process execution with redirected streams
 * 5. The relationship between file descriptors and stdio streams
 *
 * Real-world Application:
 * - Shell command: "wc p4.c > output.txt"
 * - Log file generation
 * - Batch processing output capture
 * - System administration scripts
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
    // Suppress unused parameter warnings - good practice in educational code
    (void)argc; (void)argv;
    
    /*
     * STEP 1: Create Child Process
     * ============================
     * The fork() system call creates an identical copy of the current process.
     * Both parent and child inherit the same file descriptor table initially.
     * 
     * File Descriptor Table (inherited by both processes):
     * FD 0: stdin  (keyboard input)
     * FD 1: stdout (terminal output) ← We'll redirect this
     * FD 2: stderr (terminal error output)
     */
    int rc = fork();
    if (rc < 0) {
        // Fork failed - critical system error
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) { 
        /*
         * CHILD PROCESS: Output Redirection Implementation
         * ===============================================
         * This demonstrates exactly how shell redirection works internally.
         * When you type "command > file" in bash, this is what happens.
         */
        
        /*
         * STEP 2: Close Standard Output
         * =============================
         * close(STDOUT_FILENO) closes file descriptor 1 (stdout).
         * This frees up FD 1 in the process's file descriptor table.
         * 
         * File Descriptor Table after close():
         * FD 0: stdin  (keyboard input)
         * FD 1: [AVAILABLE] ← closed, ready for reuse
         * FD 2: stderr (terminal error output)
         */
        close(STDOUT_FILENO);
        
        /*
         * STEP 3: Open Output File
         * ========================
         * open() always assigns the lowest available file descriptor.
         * Since we just closed FD 1, the new file gets assigned FD 1.
         * This means stdout now points to our file instead of the terminal!
         * 
         * Flags explained:
         * - O_CREAT: Create file if it doesn't exist
         * - O_WRONLY: Open for writing only
         * - O_TRUNC: Truncate file to zero length if it exists
         * - S_IRWXU: Give owner read/write/execute permissions (0700)
         * 
         * File Descriptor Table after open():
         * FD 0: stdin  (keyboard input)
         * FD 1: p4.output (our file) ← stdout now redirected!
         * FD 2: stderr (terminal error output)
         */
        open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        
        /*
         * STEP 4: Execute Target Command
         * ==============================
         * Now we execute the 'wc' (word count) command on p4.c.
         * Since wc writes to stdout (FD 1), and we've redirected FD 1
         * to our file, wc's output goes to p4.output instead of the terminal.
         * 
         * This is the UNIX principle: programs should be agnostic about
         * where their output goes - stdout could be terminal, file, pipe, etc.
         */
        char *myargs[3];
        myargs[0] = "wc";           // Command name
        myargs[1] = "p4.c";        // File to analyze
        myargs[2] = NULL;          // NULL-terminated argument array
        
        /*
         * execvp() replaces the current process image with 'wc'.
         * The redirected file descriptors are preserved across exec().
         * If exec() succeeds, this line never returns.
         * If exec() fails, we should handle the error (missing in this simple demo).
         */
        execvp(myargs[0], myargs);
        
        // If we reach here, exec() failed
        perror("exec failed");
        exit(1);
        
    } else {
        /*
         * PARENT PROCESS: Wait for Child Completion
         * =========================================
         * The parent waits for the child to complete the redirection and
         * command execution. This ensures proper process synchronization.
         */
        
        /*
         * wait(NULL) blocks until child terminates.
         * In a real shell, this is where the shell would:
         * 1. Check exit status
         * 2. Update job control information
         * 3. Clean up process table entries
         */
        wait(NULL);
        
        /*
         * Inform user that redirection is complete.
         * Note: Parent's stdout is still connected to terminal,
         * so this message appears on screen, not in the file.
         */
        printf("Child process completed. Check p4.output file.\n");
    }
    
    /*
     * EDUCATIONAL SUMMARY:
     * ===================
     * This program demonstrates the core mechanism behind shell redirection:
     * 1. Fork a child process
     * 2. In child: manipulate file descriptors (close + open)
     * 3. In child: exec the target command
     * 4. In parent: wait for completion
     * 
     * The key insight: File descriptors are process-specific, and the
     * close+open pattern allows us to "intercept" and redirect any
     * standard stream (stdin, stdout, stderr).
     * 
     * Try these experiments:
     * 1. Run: ./p4 && cat p4.output
     * 2. Compare with: wc p4.c
     * 3. Examine with: strace -e open,close,dup2 ./p4
     */
    return 0;
}
