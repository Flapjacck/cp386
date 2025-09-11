# Process Execution with exec()

## Overview

These programs demonstrate how to replace a process image with a new program using the `exec()` family of system calls. This is how shells and other programs launch new processes.

## Key Concepts

- **exec()**: Replaces current process image with a new program
- **execvp()**: Variant that searches PATH and takes vector of arguments
- **Process Image**: The program code and data loaded in memory
- **Program Arguments**: Command-line arguments passed to the new program
- **Path Resolution**: Finding executables in system directories

## Programs

### p3.c - Basic exec() with wc

Demonstrates replacing child process with `wc` (word count) command.

### exec_example.c - Simple exec() demonstration

Shows minimal exec() usage with `echo` command.

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

## Understanding exec()

1. **Before exec()**: Process has original program code
2. **After exec()**: Same process, but completely new program
3. **Process ID**: Remains the same
4. **File descriptors**: Usually inherited (unless marked close-on-exec)
5. **No return**: exec() never returns if successful

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

## Experiment Suggestions

1. Try exec() with different commands (ls, cat, grep)
2. Modify arguments passed to the executed program
3. Observe what happens when exec() fails (wrong program name)
4. Use strace to see the exec() system call in action
