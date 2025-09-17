/*
 * ===================================================================
 * Process Creation Demonstration - Basic fork() Example
 * ===================================================================
 * 
 * This program demonstrates the fundamental mechanism of process creation
 * in Unix-like systems using the fork() system call. It shows how one
 * process can create another process and how both processes continue
 * execution from the point of the fork() call.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Process Creation: Using fork() to create new processes
 * 2. Process Identification: PIDs (Process IDs) distinguish processes
 * 3. Parent-Child Relationships: Hierarchy of processes
 * 4. Return Values: How fork() communicates success/failure
 * 5. Code Sharing: Both processes execute the same code
 * 
 * THE fork() SYSTEM CALL:
 * ======================
 * fork() is one of the most important system calls in Unix. It:
 * 
 * 1. Creates an exact copy of the calling process
 * 2. Both processes continue execution from the fork() point
 * 3. Returns different values to distinguish parent from child:
 *    - Parent: Receives child's PID (positive number)
 *    - Child:  Receives 0
 *    - Error:  Returns -1 (fork failed)
 * 
 * PROCESS MEMORY LAYOUT AFTER fork():
 * ==================================
 * 
 * Before fork():                After fork():
 * ┌─────────────┐              ┌─────────────┐  ┌─────────────┐
 * │   Parent    │              │   Parent    │  │    Child    │
 * │   Process   │    fork()    │   Process   │  │   Process   │
 * │  (PID 1234) │    ───→      │  (PID 1234) │  │  (PID 5678) │
 * │             │              │             │  │             │
 * │ Code │ Data │              │ Code │ Data │  │ Code │ Data │
 * │ Stack│ Heap │              │ Stack│ Heap │  │ Stack│ Heap │
 * └─────────────┘              └─────────────┘  └─────────────┘
 *                                     │               │
 *                                     └─── Both continue execution
 *                                           from this point
 * 
 * References:
 * - OSTEP Chapter 5: Process API
 * - Stevens & Rago: Advanced Programming in the UNIX Environment
 * - Linux man page: fork(2)
 */

#include <stdio.h>      // For printf(), fprintf()
#include <stdlib.h>     // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>     // For fork(), getpid(), getppid()
#include <sys/types.h>  // For pid_t type definition
#include <errno.h>      // For errno and error handling
#include <string.h>     // For strerror()

/*
 * Function to display process information in a formatted way
 * This helps visualize the parent-child relationship
 */
void display_process_info(const char *role, pid_t pid, pid_t parent_pid) {
    printf("┌─────────────────────────────────────────────────────────┐\n");
    printf("│ %-20s │ PID: %-8d │ PPID: %-8d │\n", role, pid, parent_pid);
    printf("└─────────────────────────────────────────────────────────┘\n");
}

int main(int argc, char *argv[]) {
    // Suppress unused parameter warnings
    (void)argc; 
    (void)argv;
    
    /*
     * PROGRAM INITIALIZATION
     * =====================
     * Before fork(), we have only one process. Let's display its information.
     */
    pid_t original_pid = getpid();
    pid_t original_ppid = getppid();
    
    printf("=============================================================\n");
    printf("Process Creation Demonstration - Basic fork() Example\n");
    printf("=============================================================\n\n");
    
    printf("=== BEFORE fork() ===\n");
    display_process_info("Original Process", original_pid, original_ppid);
    printf("\nAbout to call fork() - this will create a child process...\n\n");
    
    /*
     * THE CRITICAL MOMENT: fork() SYSTEM CALL
     * =======================================
     * 
     * This is the key line where process creation happens.
     * After this call:
     * - Parent process: rc contains child's PID
     * - Child process:  rc contains 0
     * - Error case:     rc contains -1
     * 
     * Both processes will continue execution from this point!
     */
    printf("Calling fork()...\n");
    pid_t rc = fork();
    
    /*
     * ERROR HANDLING
     * =============
     * fork() can fail for various reasons:
     * - System has reached process limit
     * - Insufficient memory
     * - Process table is full
     */
    if (rc < 0) {
        // fork() failed - we're still in the original process
        fprintf(stderr, "ERROR: fork() failed!\n");
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        fprintf(stderr, "This means no child process was created.\n");
        exit(EXIT_FAILURE);
    }
    
    /*
     * CHILD PROCESS EXECUTION PATH
     * ===========================
     * 
     * If rc == 0, we're in the child process.
     * The child is an exact copy of the parent, but has its own PID.
     */
    else if (rc == 0) {
        // We are in the CHILD process
        pid_t child_pid = getpid();
        pid_t child_ppid = getppid();  // Parent's PID from child's perspective
        
        printf("=== CHILD PROCESS ===\n");
        display_process_info("Child Process", child_pid, child_ppid);
        
        printf("Child executing: I'm a new process!\n");
        printf("Child: My parent is PID %d\n", child_ppid);
        printf("Child: I was created by fork() in parent\n");
        printf("Child: I have my own memory space (copy of parent)\n");
        printf("Child: I will exit now...\n\n");
        
        /*
         * CHILD PROCESS TERMINATION
         * ========================
         * The child process exits here. In this simple example,
         * both parent and child exit naturally when they reach
         * the end of main(). In more complex programs, child
         * processes often exec() other programs.
         */
    }
    
    /*
     * PARENT PROCESS EXECUTION PATH
     * ============================
     * 
     * If rc > 0, we're in the parent process.
     * rc contains the PID of the newly created child.
     */
    else {
        // We are in the PARENT process
        pid_t parent_pid = getpid();
        pid_t parent_ppid = getppid();
        pid_t child_pid = rc;  // fork() returned child's PID to parent
        
        printf("=== PARENT PROCESS ===\n");
        display_process_info("Parent Process", parent_pid, parent_ppid);
        
        printf("Parent executing: I created a child!\n");
        printf("Parent: My child's PID is %d\n", child_pid);
        printf("Parent: I am the original process\n");
        printf("Parent: My child is a copy of me\n");
        printf("Parent: Both of us are running concurrently\n\n");
        
        /*
         * PROCESS RELATIONSHIP DEMONSTRATION
         * =================================
         * Show the relationship between parent and child processes.
         */
        printf("=== PROCESS RELATIONSHIP ===\n");
        printf("Parent PID: %d\n", parent_pid);
        printf("Child PID:  %d\n", child_pid);
        printf("Relationship: %d is parent of %d\n\n", parent_pid, child_pid);
    }
    
    /*
     * CONCURRENT EXECUTION DEMONSTRATION
     * =================================
     * 
     * This code runs in BOTH parent and child processes!
     * This demonstrates that both processes continue execution
     * from the fork() point.
     */
    printf("=== CONCURRENT EXECUTION ===\n");
    printf("Process %d: This message appears in both parent and child!\n", getpid());
    printf("Process %d: Both processes execute this code\n", getpid());
    printf("Process %d: This is why we check fork() return value\n", getpid());
    printf("Process %d: Terminating...\n\n", getpid());
    
    /*
     * IMPORTANT TIMING NOTE
     * ====================
     * 
     * The order of execution between parent and child is NOT guaranteed.
     * The OS scheduler decides which process runs first. You might see:
     * - Parent output first, then child
     * - Child output first, then parent  
     * - Interleaved output
     * 
     * This non-deterministic behavior is fundamental to concurrent systems.
     */
    
    printf("=== PROGRAM TERMINATION ===\n");
    printf("Process %d: Exiting with status 0\n", getpid());
    
    return EXIT_SUCCESS;
}
