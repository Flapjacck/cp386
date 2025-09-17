# Process Execution with exec() - Deep Dive into Program Loading

## Overview

These programs demonstrate the **exec()** family of system calls - one of the most fundamental operations in Unix-like operating systems. The exec() system calls completely replace the current process image with a new program while preserving the process ID. This mechanism, combined with fork(), forms the basis of how shells and process managers launch new programs.

## Core OS Concepts

### Process Image Replacement

```text
Before exec():                After exec():
┌─────────────────┐          ┌─────────────────┐
│   Original      │          │   New Program   │
│   Program       │   exec() │   (e.g., wc)    │
│   Code & Data   │  ────→   │   Code & Data   │
│   PID: 1234     │          │   PID: 1234     │ (same PID!)
│   Memory Layout │          │   Memory Layout │
└─────────────────┘          └─────────────────┘
```

### The Fork + Exec Pattern (Shell Model)

```text
Shell Process (PID: 100)
│
├─ fork() ──→ Child Process (PID: 200)
│             │
│             ├─ exec("wc") ──→ Word Count Program (PID: 200)
│             │                 │
│             │                 └─ exit() ──→ Terminated
│             │
└─ wait() ──→ Parent continues after child exits
```

## Key Concepts & Kernel Mechanics

- **exec()**: System call that loads a new program into the current process address space
- **execvp()**: PATH-searching variant that takes a vector of arguments
- **Process Image**: Complete program state including code, data, heap, and stack
- **Program Loading**: Kernel's ELF loader replaces memory contents
- **Path Resolution**: Searching through $PATH directories for executables
- **Address Space**: Virtual memory layout is completely reconstructed

## Programs & Deep Analysis

### p3.c - Fork+Exec with Word Count

Demonstrates the classic **fork+exec** pattern used by shells. The parent forks a child, the child exec's the `wc` command, and the parent waits for completion.

**Key Learning Points:**

- Fork creates identical process copy
- Child immediately exec's, replacing its image
- Parent waits for child completion
- This is exactly how bash/zsh execute commands

### exec_example.c - Multiple exec() Demonstrations

Shows various exec() family functions with comprehensive examples including:

- `execvp()` with PATH searching
- `execl()` with explicit paths  
- `execv()` with argument vectors
- Error handling and child process management

**Educational Focus:**

- Different exec() variants and their use cases
- Proper error handling in system programming
- Understanding when each exec() type is appropriate

## Memory Layout During exec()

```text
Virtual Address Space Transformation:

Before exec():              After exec():
┌─────────────────┐        ┌─────────────────┐
│     Stack       │        │     Stack       │ ← New program's stack
├─────────────────┤        ├─────────────────┤
│      ...        │        │      ...        │
├─────────────────┤        ├─────────────────┤
│     Heap        │        │     Heap        │ ← Reset for new program
├─────────────────┤        ├─────────────────┤
│  Data Segment   │ exec() │  Data Segment   │ ← New program's globals
├─────────────────┤ ────→  ├─────────────────┤
│  Text Segment   │        │  Text Segment   │ ← New program code
└─────────────────┘        └─────────────────┘
 Original Program            Target Program
   (p3 or exec_example)       (wc or echo)
```

## How to Compile and Run

### Compile

```bash
gcc -o p3 p3.c
gcc -o exec_example exec_example.c
```

### Run p3

```bash
./p3
```

### Run exec_example

```bash
./exec_example
```

### Create test files for wc

```bash
echo "Hello World" > test.txt
echo "Line 1" >> test.txt
echo "Line 2" >> test.txt
```

## Expected Output

### p3.c Output

```bash
hello world (pid:1234)
hello, I am child (pid:1235)
      22      89     584 p3.c
hello, I am parent of 1235 (wc:1235) (pid:1234)
```

### exec_example.c Output

```bash
About to execute: echo hello
hello
Child completed execution
```

## Understanding exec() - Kernel-Level Details

### Process Transformation Mechanics

1. **Before exec()**: Process has original program code loaded in memory
2. **During exec()**: Kernel's ELF loader completely replaces memory contents
3. **After exec()**: Same process, but entirely new program image
4. **Process ID**: Remains unchanged (crucial for parent's wait())
5. **File descriptors**: Usually inherited (unless marked close-on-exec)
6. **No return**: exec() never returns if successful - control never returns to caller

### What Gets Preserved vs. Replaced

**Preserved:**

- Process ID (PID)
- Parent Process ID (PPID)  
- Process Group ID
- Session ID
- Open file descriptors (unless FD_CLOEXEC set)
- Signal dispositions (some)
- Working directory
- Root directory

**Replaced:**

- Program code (text segment)
- Data segment (global variables)
- Heap memory
- Stack memory
- Process image name
- Memory mappings

## exec() Family Functions

```c
execl("/bin/ls", "ls", "-l", NULL);           // Full path, list args
execlp("ls", "ls", "-l", NULL);               // Search PATH, list args
execv("/bin/ls", argv);                       // Full path, vector args
execvp("ls", argv);                           // Search PATH, vector args
execle("/bin/ls", "ls", "-l", NULL, envp);    // With environment
execvpe("ls", argv, envp);                    // Vector + PATH + environment
```

## Learning Points

1. exec() completely replaces the process image
2. The PID stays the same, but everything else changes
3. Code after exec() only runs if exec() fails
4. This is how shells launch programs
5. fork() + exec() is the standard way to create new processes

## Additional Commands

```bash
# View available exec variants
man 3 exec

# Check word count of files
wc filename
wc -l filename  # lines only
wc -w filename  # words only
wc -c filename  # characters only

# View process execution
strace ./p3     # trace system calls

# Find executable locations
which wc
whereis wc
```

## Common exec() Use Cases

- Shells launching commands
- Process launchers
- Replacing current program with updated version
- Running external utilities from programs

## Experiment Suggestions & Advanced Learning

### Basic Experiments

1. **Different Commands**: Try exec() with various commands (ls, cat, grep, python3)
2. **Argument Variations**: Modify arguments passed to executed programs
3. **Error Conditions**: Observe behavior when exec() fails (wrong program name)
4. **System Call Tracing**: Use `strace ./p3` to see exec() system call in action

### Advanced Experiments

#### Environment Variable Handling

```bash
# Create test with environment variables
env CUSTOM_VAR="hello" ./p3
# Modify exec_example.c to use execle() and pass custom environment
```

#### File Descriptor Inheritance

```c
// Try this modification in p3.c:
int fd = open("output.txt", O_WRONLY | O_CREAT, 0644);
// exec() inherits this fd - wc output goes to file!
```

#### Signal Handling Across exec()

```c
// Add signal handler before exec() - see what survives
signal(SIGINT, SIG_IGN);  // Ignore Ctrl+C
// After exec(), check if new program inherits this
```

### Real-World Shell Implementation

This is how shells (bash, zsh, fish) actually work:

```text
Shell Command: "ls -la | grep txt"

Shell Process:
├─ Parse command line
├─ fork() ──→ Child 1: exec("/bin/ls", "ls", "-la", NULL)
├─ fork() ──→ Child 2: exec("/bin/grep", "grep", "txt", NULL)  
├─ Set up pipe between children
└─ wait() for both children to complete
```

### Performance Implications

- **exec()** is expensive - requires loading entire program from disk
- **Copy-on-Write (COW)** makes fork() cheap when followed by exec()
- **vfork()** optimizes fork+exec pattern (deprecated, use posix_spawn())
- **Path searching** with execvp() involves filesystem operations

### Security Considerations

- **Privilege escalation**: exec() can change effective user/group ID
- **setuid/setgid** programs: Special consideration for security
- **PATH injection**: Always validate executables when using execvp()
- **Argument injection**: Sanitize arguments passed to exec()

## References & Further Reading

- [OSDev Wiki - Loading ELF Files](https://wiki.osdev.org/ELF)
- OSTEP Chapter 5: Process API
- Stevens & Rago: Advanced Programming in the UNIX Environment
- `man 3 exec` - Complete exec() family documentation
- `man 2 execve` - The underlying system call
