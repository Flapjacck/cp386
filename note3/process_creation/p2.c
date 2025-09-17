/*
 * ===================================================================
 * Process Creation and Synchronization - wait() Example
 * ===================================================================
 * 
 * This program builds upon basic fork() by adding process synchronization
 * using the wait() system call. It demonstrates how parent processes can
 * wait for their children to complete, ensuring proper process cleanup
 * and preventing zombie processes.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Process Synchronization: Parent waiting for child completion
 * 2. Zombie Process Prevention: Proper child process cleanup
 * 3. Exit Status Handling: How child exit codes are communicated
 * 4. Deterministic Ordering: Controlling execution order
 * 5. Process State Transitions: Running → Zombie → Terminated
 * 
 * THE wait() SYSTEM CALL:
 * ======================
 * wait() serves several critical purposes:
 * 
 * 1. Synchronization: Parent blocks until any child terminates
 * 2. Cleanup: OS can reclaim child's process table entry
 * 3. Status Retrieval: Parent can get child's exit status
 * 4. Zombie Prevention: Prevents accumulation of zombie processes
 * 
 * PROCESS STATE DIAGRAM:
 * =====================
 * 
 * Child Process States:
 * ┌─────────┐   fork()   ┌─────────┐   exit()   ┌─────────┐   wait()   
 * │ Created │  ────────→ │ Running │ ─────────→ │ Zombie  │ ─────────→ (Cleaned up)
 * └─────────┘            └─────────┘            └─────────┘
 *                              │                      ↑
 *                              │ (still executing)    │ (terminated, waiting
 *                              ↓                      │  for parent to wait)
 *                        ┌─────────┐                  │
 *                        │ Blocked │ ─────────────────┘
 *                        └─────────┘   (I/O, sleep, etc.)
 * 
 * ZOMBIE PROCESSES:
 * ================
 * If a parent doesn't call wait():
 * - Child becomes "zombie" after exit()
 * - Zombie holds process table entry
 * - Eventually can exhaust process table
 * - wait() allows OS to fully clean up child
 * 
 * References:
 * - OSTEP Chapter 5: Process API
 * - Stevens & Rago: Advanced Programming in the UNIX Environment
 * - Linux man pages: wait(2), waitpid(2)
 */

#include <stdio.h>       // For printf(), fprintf()
#include <stdlib.h>      // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>      // For fork(), getpid(), getppid(), sleep()
#include <sys/wait.h>    // For wait(), waitpid(), WEXITSTATUS, etc.
#include <sys/types.h>   // For pid_t type definition
#include <errno.h>       // For errno and error handling
#include <string.h>      // For strerror()
#include <time.h>        // For time() - random seed

/*
 * Function to display detailed process information
 */
void display_process_info(const char *role, pid_t pid, pid_t parent_pid, const char *status) {
    printf("┌────────────────────────────────────────────────────────────────┐\n");
    printf("│ %-15s │ PID: %-8d │ PPID: %-8d │ Status: %-10s │\n", 
           role, pid, parent_pid, status);
    printf("└────────────────────────────────────────────────────────────────┘\n");
}

/*
 * Function to simulate some work in the child process
 */
void child_work_simulation() {
    printf("\n--- Child Work Simulation ---\n");
    printf("Child: Starting some important work...\n");
    
    // Simulate variable work time (1-3 seconds)
    srand(time(NULL) + getpid());  // Seed with current time + PID
    int work_time = 1 + (rand() % 3);
    
    printf("Child: Working for %d seconds...\n", work_time);
    
    for (int i = 1; i <= work_time; i++) {
        printf("Child: Work progress... %d/%d seconds\n", i, work_time);
        sleep(1);  // Simulate 1 second of work
    }
    
    printf("Child: Work completed successfully!\n");
    printf("Child: Preparing to exit...\n");
}

/*
 * Function to analyze wait() return values
 */
void analyze_wait_result(pid_t child_pid, pid_t wait_result, int status) {
    printf("\n=== wait() Analysis ===\n");
    printf("Expected child PID: %d\n", child_pid);
    printf("wait() returned:    %d\n", wait_result);
    
    if (wait_result == child_pid) {
        printf("✓ Correct: wait() returned the expected child PID\n");
    } else if (wait_result > 0) {
        printf("⚠ Unexpected: wait() returned different child PID\n");
    } else {
        printf("✗ Error: wait() failed with return value %d\n", wait_result);
        return;
    }
    
    // Analyze child exit status
    printf("\nChild Exit Status Analysis:\n");
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("✓ Child exited normally with code: %d\n", exit_code);
        if (exit_code == 0) {
            printf("  └─ SUCCESS: Child completed successfully\n");
        } else {
            printf("  └─ ERROR: Child reported failure (code %d)\n", exit_code);
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
    pid_t original_ppid = getppid();
    
    printf("=============================================================\n");
    printf("Process Creation and Synchronization - wait() Example\n");
    printf("=============================================================\n\n");
    
    printf("=== BEFORE fork() ===\n");
    display_process_info("Original Process", original_pid, original_ppid, "Running");
    printf("\nThis program demonstrates:\n");
    printf("1. Process creation with fork()\n");
    printf("2. Parent-child synchronization with wait()\n");
    printf("3. Proper zombie process cleanup\n");
    printf("4. Exit status communication\n\n");
    
    printf("About to call fork() to create child process...\n\n");
    
    /*
     * PROCESS CREATION
     * ===============
     */
    printf("Calling fork()...\n");
    pid_t rc = fork();
    
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
        pid_t child_ppid = getppid();
        
        printf("=== CHILD PROCESS CREATED ===\n");
        display_process_info("Child Process", child_pid, child_ppid, "Running");
        
        printf("Child: I am the child process!\n");
        printf("Child: My parent (PID %d) will wait for me to finish\n", child_ppid);
        printf("Child: I will do some work then exit\n");
        
        /*
         * CHILD WORK SIMULATION
         * ====================
         * The child does some work to simulate a real process.
         * This makes the wait() demonstration more realistic.
         */
        child_work_simulation();
        
        /*
         * CHILD EXIT
         * ==========
         * Child exits with status 0 (success).
         * This status will be retrieved by the parent via wait().
         */
        printf("\nChild: Exiting with status 0 (success)\n");
        printf("Child: Parent will receive this exit status via wait()\n");
        
        exit(EXIT_SUCCESS);  // Child terminates here
    }
    
    /*
     * PARENT PROCESS EXECUTION PATH
     * ============================
     */
    else {
        // We are in the PARENT process
        pid_t parent_pid = getpid();
        pid_t parent_ppid = getppid();
        pid_t child_pid = rc;  // fork() returned child's PID
        
        printf("=== PARENT PROCESS CONTINUES ===\n");
        display_process_info("Parent Process", parent_pid, parent_ppid, "Running");
        
        printf("Parent: Child process created with PID %d\n", child_pid);
        printf("Parent: Child is now running concurrently with me\n");
        printf("Parent: I will now wait for child to complete...\n\n");
        
        /*
         * PROCESS SYNCHRONIZATION WITH wait()
         * ==================================
         * 
         * This is the key difference from p1.c!
         * The parent calls wait() to:
         * 1. Block until child terminates
         * 2. Retrieve child's exit status
         * 3. Allow OS to clean up child's process table entry
         */
        printf("--- Parent Waiting for Child ---\n");
        printf("Parent: Calling wait() - this will block until child exits\n");
        printf("Parent: While waiting, child is doing its work...\n");
        
        int status;
        pid_t wc = wait(&status);  // Block until any child terminates
        
        /*
         * WAIT COMPLETED - CHILD HAS TERMINATED
         * ====================================
         */
        printf("\n--- Child Process Completed ---\n");
        printf("Parent: wait() returned! Child has terminated.\n");
        
        /*
         * ANALYZE WAIT RESULTS
         * ===================
         */
        analyze_wait_result(child_pid, wc, status);
        
        /*
         * DEMONSTRATE SUCCESSFUL SYNCHRONIZATION
         * =====================================
         */
        printf("\n=== SYNCHRONIZATION SUCCESS ===\n");
        printf("Parent: I successfully waited for child %d\n", child_pid);
        printf("Parent: Child's exit was properly handled\n");
        printf("Parent: No zombie process was created\n");
        printf("Parent: This is proper process management!\n");
        
        /*
         * SHOW FINAL PROCESS STATE
         * =======================
         */
        printf("\n=== FINAL STATE ===\n");
        display_process_info("Parent Process", parent_pid, parent_ppid, "Running");
        printf("Child Process:  TERMINATED (properly cleaned up)\n");
    }
    
    /*
     * PROGRAM COMPLETION
     * =================
     * 
     * Only the parent reaches this point because the child
     * called exit() earlier. This ensures clean termination.
     */
    printf("\n=== PROGRAM TERMINATION ===\n");
    printf("Process %d: All child processes completed\n", getpid());
    printf("Process %d: No zombie processes remain\n", getpid());
    printf("Process %d: Exiting cleanly\n", getpid());
    
    return EXIT_SUCCESS;
}
