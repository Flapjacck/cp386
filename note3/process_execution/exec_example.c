/*
 * ===================================================================
 * Advanced Process Execution Examples - exec() Family Showcase
 * ===================================================================
 * 
 * This program provides multiple examples of the exec() family of system
 * calls, demonstrating different ways to execute external programs and
 * showing the versatility of process execution in Unix-like systems.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Multiple exec() variants (execl, execv, execvp, etc.)
 * 2. Argument passing techniques
 * 3. Environment variable handling
 * 4. PATH search functionality
 * 5. Error handling for exec() failures
 * 6. Shell command execution patterns
 * 
 * THE exec() FAMILY OVERVIEW:
 * ==========================
 * 
 * exec() Function Variants:
 * ┌─────────┬────────────┬─────────────┬──────────────────────────────┐
 * │Function │ Arguments  │Environment  │ Description                  │
 * ├─────────┼────────────┼─────────────┼──────────────────────────────┤
 * │execl()  │ List       │ Inherit     │ Arguments as separate params │
 * │execv()  │ Vector     │ Inherit     │ Arguments as array           │
 * │execle() │ List       │ Explicit    │ List + environment array     │
 * │execve() │ Vector     │ Explicit    │ Vector + environment array   │
 * │execlp() │ List       │ Inherit     │ List + PATH search           │
 * │execvp() │ Vector     │ Inherit     │ Vector + PATH search         │
 * └─────────┴────────────┴─────────────┴──────────────────────────────┘
 * 
 * MEMORY BEHAVIOR:
 * ===============
 * exec() completely replaces the process image:
 * - Same PID, different program
 * - New code, data, heap, stack
 * - File descriptors can be preserved or closed
 * - Process attributes (PID, PPID, etc.) remain
 * 
 * References:
 * - OSTEP Chapter 5: Process API
 * - Stevens & Rago: Advanced Programming in UNIX Environment
 * - POSIX.1-2008 Standard: exec() specifications
 */

#include <stdio.h>       // For printf(), fprintf()
#include <stdlib.h>      // For exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>      // For exec() family, fork(), getpid()
#include <sys/wait.h>    // For wait() and status macros
#include <errno.h>       // For errno and error handling
#include <string.h>      // For strerror()
#include <sys/types.h>   // For pid_t

/*
 * Global counter to track example numbers
 */
static int example_counter = 1;

/*
 * Function to display example headers
 */
void print_example_header(const char *title, const char *description) {
    printf("\n" "═" "═" "═" " Example %d: %s " "═" "═" "═\n", example_counter++, title);
    printf("Description: %s\n", description);
    printf("Process PID: %d\n", getpid());
    printf("─────────────────────────────────────────────────────\n");
}

/*
 * Example 1: Simple execvp() with PATH search
 * ===========================================
 * This is the most commonly used exec() variant in shell-like programs.
 * It searches for the program in PATH and takes arguments as an array.
 */
void example_execvp() {
    print_example_header("execvp() with PATH search", 
                         "Execute 'echo' command with PATH lookup");
    
    char *argv[4];
    argv[0] = "echo";                    // Program name
    argv[1] = "Hello from execvp()!";    // First argument
    argv[2] = "Process execution works!"; // Second argument  
    argv[3] = NULL;                      // NULL terminator (required)
    
    printf("About to execute: echo \"Hello from execvp()!\" \"Process execution works!\"\n");
    printf("Using execvp() - will search PATH for 'echo' program\n");
    printf("Arguments passed as array of strings\n\n");
    
    // execvp() searches PATH and uses vector (array) of arguments
    execvp("echo", argv);
    
    // This line only executes if exec() failed
    fprintf(stderr, "ERROR: execvp() failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

/*
 * Example 2: execl() with explicit path and argument list
 * ======================================================
 * Uses absolute path and passes arguments as separate parameters.
 */
void example_execl() {
    print_example_header("execl() with explicit path", 
                         "Execute '/bin/ls' with argument list");
    
    printf("About to execute: /bin/ls -l -a\n");
    printf("Using execl() - explicit path, arguments as separate parameters\n");
    printf("Will list files in long format including hidden files\n\n");
    
    // execl() takes arguments as separate parameters, ending with NULL
    execl("/bin/ls",        // Full path to program
          "ls",             // argv[0] (program name)  
          "-l",             // argv[1] (long format)
          "-a",             // argv[2] (show hidden files)
          (char *)NULL);    // NULL terminator
    
    fprintf(stderr, "ERROR: execl() failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

/*
 * Example 3: execv() with vector of arguments
 * ==========================================
 * Similar to execl() but takes arguments as an array instead of separate parameters.
 */
void example_execv() {
    print_example_header("execv() with argument vector", 
                         "Execute '/bin/date' with vector arguments");
    
    char *argv[3];
    argv[0] = "date";              // Program name
    argv[1] = "+%Y-%m-%d %H:%M:%S"; // Format string for date
    argv[2] = NULL;               // NULL terminator
    
    printf("About to execute: /bin/date \"+%%Y-%%m-%%d %%H:%%M:%%S\"\n");
    printf("Using execv() - explicit path, arguments as array\n");
    printf("Will display current date and time in ISO format\n\n");
    
    execv("/bin/date", argv);
    
    fprintf(stderr, "ERROR: execv() failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

/*
 * Example 4: execle() with custom environment
 * ==========================================
 * Demonstrates passing a custom environment to the executed program.
 */
void example_execle() {
    print_example_header("execle() with custom environment", 
                         "Execute '/usr/bin/env' with custom environment");
    
    // Create custom environment variables
    char *envp[4];
    envp[0] = "CUSTOM_VAR=Hello from execle!";
    envp[1] = "DEMO_MODE=advanced_exec_example";  
    envp[2] = "PATH=/bin:/usr/bin";  // Minimal PATH
    envp[3] = NULL;                 // NULL terminator
    
    printf("About to execute: /usr/bin/env\n");
    printf("Using execle() - custom environment variables\n");
    printf("Custom environment includes:\n");
    printf("  CUSTOM_VAR=Hello from execle!\n");
    printf("  DEMO_MODE=advanced_exec_example\n");
    printf("  PATH=/bin:/usr/bin\n\n");
    
    execle("/usr/bin/env",    // Program path
           "env",             // argv[0]
           (char *)NULL,      // No additional arguments
           envp);             // Custom environment
    
    fprintf(stderr, "ERROR: execle() failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

/*
 * Example 5: Complex command with multiple arguments
 * =================================================
 * Demonstrates executing a more complex command with multiple arguments.
 */
void example_complex_command() {
    print_example_header("Complex command execution", 
                         "Execute 'wc' to count lines, words, and characters");
    
    char *argv[4];
    argv[0] = "wc";                      // Program name
    argv[1] = "-l";                      // Count lines only
    argv[2] = "exec_example.c";          // File to analyze (this source file)
    argv[3] = NULL;                      // NULL terminator
    
    printf("About to execute: wc -l exec_example.c\n");
    printf("Using execvp() to count lines in this source file\n");
    printf("This demonstrates file processing with exec()\n\n");
    
    execvp("wc", argv);
    
    fprintf(stderr, "ERROR: execvp() failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

/*
 * Function to create child process and run specific example
 */
void run_example_in_child(void (*example_func)(void), const char *example_name) {
    printf("\n" "┌" "─" " Starting %s " "─" "┐\n", example_name);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        fprintf(stderr, "ERROR: fork() failed for %s: %s\n", 
                example_name, strerror(errno));
        return;
    }
    
    if (pid == 0) {
        // Child process - run the example
        example_func();
        // Should never reach here if exec() succeeds
        exit(EXIT_FAILURE);
    } else {
        // Parent process - wait for child and analyze result
        int status;
        waitpid(pid, &status, 0);
        
        printf("└─ %s completed ", example_name);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                printf("successfully ✓\n");
            } else {
                printf("with error (exit code: %d) ✗\n", exit_code);
            }
        } else if (WIFSIGNALED(status)) {
            printf("terminated by signal %d ✗\n", WTERMSIG(status));
        }
    }
}

/*
 * Function to display educational summary
 */
void display_educational_summary() {
    printf("\n" "═" "═" "═" " Educational Summary " "═" "═" "═\n");
    printf("Key Learning Points:\n");
    printf("1. exec() replaces process image but keeps same PID\n");
    printf("2. execvp() is most common - searches PATH, takes array\n");
    printf("3. execl() takes arguments as separate parameters\n");
    printf("4. execle() allows custom environment variables\n");
    printf("5. All exec() variants replace calling process completely\n");
    printf("6. If exec() succeeds, code after it never executes\n");
    printf("7. fork() + exec() is the Unix way to run new programs\n\n");
    
    printf("Real-world applications:\n");
    printf("• Shells executing user commands\n");
    printf("• Build systems running compilers\n");
    printf("• Web servers launching CGI scripts\n");
    printf("• Process managers starting services\n");
    printf("• Container orchestration systems\n");
}

int main(int argc, char *argv[]) {
    // Suppress unused parameter warnings
    (void)argc; 
    (void)argv;
    
    printf("=============================================================\n");
    printf("Advanced Process Execution Examples - exec() Family Showcase\n");
    printf("=============================================================\n");
    printf("Parent Process PID: %d\n", getpid());
    printf("This program demonstrates various exec() system calls\n");
    
    /*
     * Run each example in a separate child process
     * This allows us to demonstrate multiple exec() calls
     * since exec() replaces the process image
     */
    
    run_example_in_child(example_execvp, "execvp() Example");
    run_example_in_child(example_execl, "execl() Example");
    run_example_in_child(example_execv, "execv() Example");
    run_example_in_child(example_execle, "execle() Example");
    run_example_in_child(example_complex_command, "Complex Command Example");
    
    /*
     * Display educational content
     */
    display_educational_summary();
    
    printf("=============================================================\n");
    printf("All examples completed! Parent process %d terminating.\n", getpid());
    printf("=============================================================\n");
    
    return EXIT_SUCCESS;
}
