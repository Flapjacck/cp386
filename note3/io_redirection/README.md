# I/O Redirection - Deep Dive into Unix Stream Manipulation

## Overview

These programs demonstrate **I/O redirection** - one of the most powerful features of Unix-like operating systems. I/O redirection allows programs to read input from files instead of keyboards, and write output to files instead of terminals, without the programs knowing the difference. This is the foundation of shell operators like `>`, `<`, `>>`, and `|`.

## Core OS Concepts & File Descriptor Theory

### The Unix "Everything is a File" Philosophy

In Unix systems, **everything is treated as a file**:

```text
File Descriptor Table (per process):
┌────┬─────────────────────────────────┐
│ FD │ Points to                       │
├────┼─────────────────────────────────┤
│ 0  │ stdin  (keyboard/input source)  │
│ 1  │ stdout (terminal/output dest)   │
│ 2  │ stderr (terminal/error dest)    │
│ 3  │ [available for new files]       │
│ 4  │ [available for new files]       │
│...│ [more file descriptors]         │
└────┴─────────────────────────────────┘
```

### Redirection Mechanism

```text
Before Redirection:          After Output Redirection (>):
                            
Process                      Process
┌─────────────┐             ┌─────────────┐
│   Program   │             │   Program   │
│             │             │             │
│ write(1, …) │────stdout──→ │ write(1, …) │────┐
└─────────────┘   Terminal   └─────────────┘    │
                                               │
                                               ▼
                                           ┌────────┐
                                           │ File   │
                                           │ output │
                                           └────────┘
```

### Key File Descriptor Concepts

- **File Descriptors (FDs)**: Small integers that identify open files
- **Standard Streams**: 0=stdin, 1=stdout, 2=stderr (POSIX standard)
- **Inheritance**: Child processes inherit parent's file descriptor table
- **Lowest Available**: open() always assigns lowest available FD number
- **Redirection**: Manipulating which files FDs point to

## Programs & Deep Analysis

### p4.c - Output Redirection Implementation

**Demonstrates:** How `wc p4.c > p4.output` works internally

**Core Technique:**

1. Fork child process (inherits FD table)
2. Close stdout (FD 1) in child
3. Open output file (gets FD 1)
4. exec() target program
5. Target program writes to "stdout" (actually our file)

**Shell Command Equivalent:** `wc p4.c > p4.output`

### redirect_demo.c - Comprehensive I/O Examples

**Demonstrates:**

- File descriptor inheritance across fork()
- Input redirection (< operator simulation)
- Output redirection techniques
- Process synchronization with I/O streams

**Educational Value:** Shows both input and output redirection patterns

## File Descriptor Manipulation Techniques

### Output Redirection Pattern (> operator)

```c
close(STDOUT_FILENO);                              // Close stdout (FD 1)
int fd = open("output.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644); // Gets FD 1
// All stdout writes now go to output.txt
```

### Input Redirection Pattern (< operator)

```c
close(STDIN_FILENO);                               // Close stdin (FD 0)  
int fd = open("input.txt", O_RDONLY);              // Gets FD 0
// All stdin reads now come from input.txt
```

### Error Redirection Pattern (2> operator)

```c
close(STDERR_FILENO);                              // Close stderr (FD 2)
int fd = open("error.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644); // Gets FD 2
// All error output now goes to error.txt
```

## Advanced Redirection Scenarios

### Append Redirection (>> operator)

```text
Shell: command >> file.txt
Implementation:
┌─────────────────────────────────────┐
│ close(STDOUT_FILENO);               │
│ open("file.txt", O_WRONLY|O_APPEND);│ ← Key: O_APPEND flag
└─────────────────────────────────────┘
```

### Combined Redirection

```text
Shell: command < input.txt > output.txt 2> error.txt
Implementation:
┌─────────────────────────────────────┐
│ close(0); open("input.txt", O_RDONLY);    │
│ close(1); open("output.txt", O_WRONLY);   │
│ close(2); open("error.txt", O_WRONLY);    │ 
│ exec(command);                       │
└─────────────────────────────────────┘
```

## How to Compile and Run

### Compile Programs

```bash
gcc -o p4 p4.c
gcc -o redirect_demo redirect_demo.c
```

### Run Output Redirection Demo

```bash
./p4
cat p4.output  # View redirected wc output
```

### Run Comprehensive Demo

```bash
./redirect_demo
cat input.txt  # View created input file
```

### Compare with Shell Redirection

```bash
# Our implementation vs shell implementation
./p4                    # Our redirection
wc p4.c > shell.output  # Shell redirection
diff p4.output shell.output  # Should be identical!
```

## Expected Output Analysis

### p4.c Output

**Terminal Output:**
```text
Child process completed. Check p4.output file.
```

**File Contents (p4.output):**
```text
     153     680    4891 p4.c
```

**Analysis:** The word count output went to the file instead of terminal, proving redirection worked.

### redirect_demo.c Output

```text
=== I/O Redirection Comprehensive Demo ===

File descriptor demo:
Demonstrating FD inheritance across fork()...
hello world
Input redirection completed.

=== Demo Summary ===
1. File descriptors are inherited by child processes
2. close() + open() sequence redirects I/O streams  
3. Programs are unaware of redirection - they just use stdin/stdout
4. This is how shells implement < and > operators

Files created: input.txt
Experiment: Try 'cat input.txt' to see the test content.
```

## System-Level Analysis with strace

### Tracing File Operations

```bash
strace -e open,close,dup2 ./p4 2>&1 | grep -E "(open|close)"
```

**Expected Trace:**
```text
close(1)                                = 0
open("./p4.output", O_WRONLY|O_CREAT|O_TRUNC, 0700) = 1
```

This shows exactly how our redirection works at the system call level.

## Shell Implementation Context

### How Bash Implements Redirection

```text
User types: "wc file.txt > output.txt"

Shell Process:
├─ Parse command line and detect > operator
├─ fork() child process
├─ In child:
│  ├─ close(STDOUT_FILENO)
│  ├─ open("output.txt", O_WRONLY|O_CREAT|O_TRUNC)
│  └─ execvp("wc", ["wc", "file.txt", NULL])
└─ In parent: wait() for child completion
```

### Advanced Shell Features

- **Pipes (`|`)**: Connect stdout of one process to stdin of another
- **Here Documents (`<<`)**: Inline input redirection
- **Process Substitution (`<()`)**: Treat command output as file
- **Tee (`|tee`)**: Split output to multiple destinations

## File Permission & Security Considerations

### File Creation Permissions

```c
// Secure file creation
open("output.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
//                                            ^^^^
//                                     Owner: rw-, Group: r--, Other: r--
```

### Common Security Issues

- **Path Traversal:** `../../../etc/passwd` in filenames
- **Race Conditions:** TOCTOU (Time-of-Check-Time-of-Use) attacks  
- **Privilege Escalation:** Writing to privileged directories
- **Resource Exhaustion:** Creating massive output files

## Advanced Experiments & Learning

### Experiment 1: Error Stream Redirection

```c
// Modify p4.c to redirect stderr instead of stdout
close(STDERR_FILENO);
open("error.log", O_CREAT|O_WRONLY|O_TRUNC, 0644);
// Now all error messages go to error.log
```

### Experiment 2: Multiple Output Streams

```c
// Create your own 'tee' command
// Read from stdin, write to both stdout and file
```

### Experiment 3: Bidirectional Redirection

```c
// Implement command that reads from file and writes to another file
close(0); open("input.txt", O_RDONLY);     // stdin from file
close(1); open("output.txt", O_WRONLY);    // stdout to file  
exec("tr", "tr", "a-z", "A-Z", NULL);      // Convert to uppercase
```

### Experiment 4: Shell-like Implementation

```c
// Create a mini-shell that supports:
// - Simple commands
// - Output redirection (>)
// - Input redirection (<)
// - Error redirection (2>)
```

## Real-World Applications

### System Administration

```bash
# Log rotation with redirection
./monitor_system > /var/log/system_$(date +%Y%m%d).log 2>&1
```

### Data Processing Pipelines

```bash
# ETL pipeline using redirection
./extract_data < source.csv > clean_data.csv 2> error.log
```

### Automated Testing

```bash
# Capture test output and errors separately
./run_tests > test_results.txt 2> test_errors.txt
```

## Performance & Efficiency Considerations

### Buffering Behavior

- **Full Buffering:** When stdout goes to file (4KB+ buffers)
- **Line Buffering:** When stdout goes to terminal (newline triggers flush)
- **No Buffering:** stderr is typically unbuffered

### System Call Overhead

- **Redirection Cost:** Minimal - just file descriptor table manipulation
- **I/O Performance:** Depends on underlying storage (SSD vs HDD vs network)
- **Memory Usage:** File buffers in kernel space

## References & Further Reading

- [OSDev Wiki - File Systems](https://wiki.osdev.org/File_Systems)
- OSTEP Chapter 39: Files and Directories  
- Stevens & Rago: Advanced Programming in the UNIX Environment (Chapter 3)
- POSIX.1-2017 Standard: File Descriptor Operations
- `man 2 open` - System call documentation
- `man 3 stdio` - Standard I/O library
- `man 1 bash` - Shell redirection operators
