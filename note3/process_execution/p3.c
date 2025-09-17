/*
 * ===================================================================
 * Process Execution Demonstration - fork() + exec() Pattern
 * ===================================================================
 * 
 * This program demonstrates the classic Unix pattern of process creation
 * and execution: fork() followed by exec(). This combination allows a
 * process to create a child and have that child run a completely different
 * program, which is the foundation of how shells work and how programs
 * are launched in Unix-like systems.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Process Creation: Using fork() to create a child process
 * 2. Program Execution: Using exec() to replace process image
 * 3. Process Synchronization: Parent waits for child completion
 * 4. Command Execution: How shells execute external programs
 * 5. File Processing: Demonstrating with 'wc' (word count) utility
 * 
 * THE exec() FAMILY:
 * =================
 * exec() doesn't create a new process - it replaces the current process
 * image with a new program. The process keeps the same PID, but runs
 * different code.
 * 
 * Common exec() variants:
 * - execl()  : List of arguments
 * - execv()  : Vector (array) of arguments  
 * - execle() : List + environment
 * - execve() : Vector + environment
 * - execlp() : List + PATH search
 * - execvp() : Vector + PATH search (used here)
 * 
 * FORK + EXEC PATTERN:
 * ===================
 * This is how shells work:
 * 
 * 1. Shell receives command: "wc p3.c"
 * 2. Shell calls fork() to create child process
 * 3. Child calls exec() to run "wc" program
 * 4. Parent (shell) waits for child to complete
 * 5. Shell displays prompt for next command
 * 
 * MEMORY TRANSFORMATION:
 * =====================
 * 
 * After fork():                    After exec():
 * ┌─────────────────┐             ┌─────────────────┐
 * │     Parent      │             │     Parent      │
 * │   (p3 code)     │             │   (p3 code)     │  
 * │   PID: 1234     │             │   PID: 1234     │
 * └─────────────────┘             └─────────────────┘
 * ┌─────────────────┐             ┌─────────────────┐
 * │     Child       │   exec()    │     Child       │
 * │   (p3 code)     │   ────→     │   (wc code)     │ ← Same PID, new program
 * │   PID: 5678     │             │   PID: 5678     │
 * └─────────────────┘             └─────────────────┘
 * 
 * References:
 * - OSTEP Chapter 5: Process API
 * - Stevens & Rago: Advanced Programming in the UNIX Environment
 * - Linux man pages: exec(3), execvp(3)
 */

#include <stdio.h>       // For printf(), fprintf()
#include <stdlib.h>      // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>      // For fork(), execvp(), getpid()
#include <string.h>      // For string manipulation
#include <sys/wait.h>    // For wait() and status macros
#include <errno.h>       // For errno and error handling
#include <sys/types.h>   // For pid_t type

/*
 * Function to display process information
 */
void display_process_info(const char *role, pid_t pid, const char *program) {
    printf("┌─────────────────────────────────────────────────────────────┐\n");
    printf("│ %-15s │ PID: %-8d │ Program: %-15s │\n", role, pid, program);
    printf("└─────────────────────────────────────────────────────────────┘\n");
}

/*
 * Function to analyze child exit status
 */
void analyze_child_exit(pid_t child_pid, int status) {
    printf("\n=== Child Process Analysis ===\n");
    printf("Child PID: %d\n", child_pid);
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("✓ Child exited normally with code: %d\n", exit_code);
        
        if (exit_code == 0) {
            printf("  └─ SUCCESS: Command completed successfully\n");
        } else {
            printf("  └─ ERROR: Command failed (exit code %d)\n", exit_code);
        }
    } else if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        printf("✗ Child terminated by signal: %d\n", signal);
    } else {
        printf("? Child status unclear\n");
    }
}

int main(int argc, char *argv[]) {
    // Suppress unused parameter warnings
    (void)argc; 
    (void)argv;
    
    /*
     * PROGRAM INITIALIZATION
     * =====================
     */
    pid_t original_pid = getpid();
    
    printf("=============================================================\n");
    printf("Process Execution Demonstration - fork() + exec() Pattern\n");
    printf("=============================================================\n\n");
    
    display_process_info("Original Process", original_pid, "p3 (this program)");
    
    printf("\nThis program demonstrates:\n");
    printf("1. Process creation with fork()\n");
    printf("2. Program execution with exec()\n");
    printf("3. Child running different program (wc)\n");
    printf("4. Parent-child synchronization\n\n");
    
    printf("About to fork() and exec() the 'wc' command on this source file...\n\n");
    
    /*
     * PROCESS CREATION
     * ===============
     */
    printf("=== STEP 1: Creating Child Process ===\n");
    int rc = fork();
    
    /*
     * ERROR HANDLING
     * =============
     */
    if (rc < 0) {
        fprintf(stderr, "ERROR: fork() failed!\n");
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    /*
     * CHILD PROCESS EXECUTION PATH
     * ===========================
     */
    else if (rc == 0) {
        // We are in the CHILD process
        pid_t child_pid = getpid();
        
        printf("=== STEP 2: Child Process Created ===\n");
        display_process_info("Child Process", child_pid, "p3 (about to exec)");
        
        printf("Child: I am about to exec() the 'wc' command\n");
        printf("Child: This will replace my process image\n");
        printf("Child: After exec(), I will be running 'wc' instead of p3\n");
        printf("Child: My PID (%d) will stay the same\n\n", child_pid);
        
        /*
         * PROGRAM EXECUTION WITH exec()
         * =============================
         * 
         * We use execvp() to run the 'wc' (word count) command.
         * execvp() will:
         * 1. Search for 'wc' in the PATH
         * 2. Replace this process's memory image with 'wc'
         * 3. Pass the arguments to 'wc'
         * 4. Start execution from 'wc's main() function
         * 
         * Important: If exec() succeeds, the code after it NEVER executes
         * because the process is now running a different program!
         */
        printf("=== STEP 3: Executing 'wc' Command ===\n");
        printf("Child: Calling execvp() with arguments: wc p3.c\n");
        printf("Child: Process image will be replaced...\n\n");
        
        // Prepare arguments for wc command
        char *myargs[3];
        myargs[0] = "wc";        // Program name (what to execute)
        myargs[1] = "p3.c";     // Argument: count words in this source file
        myargs[2] = NULL;       // NULL terminator (required)
        
        /*
         * THE CRITICAL EXEC() CALL
         * ========================
         * After this point, if successful, this process will be running
         * the 'wc' program instead of our p3 program.
         */
        execvp(myargs[0], myargs);
        
        /*
         * EXEC ERROR HANDLING
         * ==================
         * This code only executes if exec() FAILED!
         * If exec() succeeds, this process is replaced and this
         * code is never reached.
         */
        fprintf(stderr, "ERROR: exec() failed!\n");
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        fprintf(stderr, "Command attempted: %s %s\n", myargs[0], myargs[1]);
        printf("Child: This message means exec() failed!\n");
        
        exit(EXIT_FAILURE);  // Child exits with error status
    }
    
    /*
     * PARENT PROCESS EXECUTION PATH
     * ============================
     */
    else {
        // We are in the PARENT process
        pid_t parent_pid = getpid();
        pid_t child_pid = rc;
        
        printf("=== STEP 4: Parent Process Continues ===\n");
        display_process_info("Parent Process", parent_pid, "p3 (original)");
        
        printf("Parent: Child process %d was created\n", child_pid);
        printf("Parent: Child is now executing 'wc' command\n");
        printf("Parent: I will wait for child to complete...\n\n");
        
        /*
         * PROCESS SYNCHRONIZATION
         * ======================
         * Parent waits for child to finish executing the 'wc' command.
         */
        printf("=== STEP 5: Waiting for Child Completion ===\n");
        printf("Parent: Calling wait() - blocking until child exits\n");
        printf("Parent: Child output will appear below:\n");
        printf("────────────────────────────────────────────────────────────\n");
        
        int status;
        int wc = wait(&status);  // Wait for child to complete
        
        printf("────────────────────────────────────────────────────────────\n");
        printf("Parent: Child has completed execution!\n");
        
        /*
         * ANALYZE RESULTS
         * ==============
         */
        analyze_child_exit(wc, status);
        
        printf("\n=== STEP 6: Final Results ===\n");
        printf("Parent: Successfully demonstrated fork() + exec() pattern\n");
        printf("Parent: Child process %d ran 'wc' command and exited\n", child_pid);
        printf("Parent: This is how shells execute external commands\n");
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Parent: The 'wc' command executed successfully!\n");
        }
    }
    
    /*
     * PROGRAM COMPLETION
     * =================
     * Only the parent reaches this point.
     */
    printf("\n=== Program Termination ===\n");
    printf("Process %d: Demonstrating how Unix shells work!\n", getpid());
    printf("Process %d: fork() creates child, exec() runs new program\n", getpid());
    printf("Process %d: This pattern is used billions of times daily\n", getpid());
    
    return EXIT_SUCCESS;
}
