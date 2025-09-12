# Pipes and Inter-Process Communication

## Overview

These programs demonstrate how to use pipes for inter-process communication (IPC). Pipes allow processes to communicate by connecting the output of one process to the input of another, which is the foundation of Unix shell pipelines.

## Key Concepts

- **Pipes**: Communication channels between processes
- **Anonymous Pipes**: Created with pipe() system call, exist only during program execution
- **File Descriptors**: Pipes create two FDs - one for reading, one for writing
- **Producer-Consumer**: One process writes data, another reads it
- **Shell Pipelines**: How commands like `ls | grep txt` work

## Programs

### pipe_demo.c - Basic Pipe (echo ... | wc)

Demonstrates the classic pipe pattern - parent writes data, child runs wc to count it.

### advanced_pipes.c - Advanced Pipe Examples

Shows simple and bidirectional pipe communication patterns.

## How to Compile and Run

### Compile

```bash
gcc -o pipe_demo pipe_demo.c
gcc -o advanced_pipes advanced_pipes.c
```

### Run pipe_demo

```bash
./pipe_demo
```

### Run advanced_pipes

```bash
./advanced_pipes
```

### Compare with shell pipes

```bash
# Shell equivalent of pipe_demo
echo -e "hello world\nthis is a test\nof the pipe system" | wc

# Other useful pipe examples
ls -la | wc -l          # Count files in directory
ps aux | grep your_name # Find your processes
cat file.txt | sort     # Sort file contents
```

## Expected Output

### pipe_demo.c

```bash
      3      11      46
```

### advanced_pipes.c

```bash
      3      11      46
```

Simple pipe demo (echo | wc):

```bash
      3       6      38

Bidirectional pipe demo:
Child received: Hello from parent!
Parent received: Hello from child!
```

## How Pipes Work

1. **Create pipe**: `pipe()` creates two file descriptors
2. **Fork process**: Both parent and child inherit the pipe FDs
3. **Close unused ends**: Each process closes the FD it doesn't need
4. **Redirect I/O**: Use `dup()` or `dup2()` to redirect stdin/stdout
5. **Communicate**: Write to one end, read from the other

## Pipe Mechanics

```c
int pipefd[2];
pipe(pipefd);           // pipefd[0] = read end, pipefd[1] = write end

// In writer process:
close(pipefd[0]);       // Close read end
write(pipefd[1], data, size);
close(pipefd[1]);       // Close write end (signals EOF)

// In reader process:
close(pipefd[1]);       // Close write end
read(pipefd[0], buffer, size);
close(pipefd[0]);       // Close read end
```

## Common Pipe Patterns

### Simple Pipeline (A | B)

```c
pipe(p);
if (fork() == 0) {
    dup2(p[0], 0);      // stdin = pipe read
    close(p[0]); close(p[1]);
    exec("program_B");
} else {
    dup2(p[1], 1);      // stdout = pipe write
    close(p[0]); close(p[1]);
    exec("program_A");
}
```

### Multiple Stage Pipeline (A | B | C)

Requires multiple pipes and processes - each intermediate process reads from one pipe and writes to another.

## Learning Points

1. Pipes enable powerful process composition
2. Data flows in one direction (unless using multiple pipes)
3. Closing the write end signals EOF to the reader
4. This is how shell command chaining works
5. Pipes are the foundation of Unix philosophy: "small tools that work together"

## Advanced Pipe Commands

```bash
# Create named pipes (FIFOs)
mkfifo mypipe
echo "hello" > mypipe &
cat < mypipe

# Monitor pipe usage
lsof | grep pipe

# Complex pipelines
ps aux | grep -v grep | grep your_name | wc -l

# Pipeline with multiple stages
cat /proc/cpuinfo | grep processor | wc -l

# Save intermediate results
cat file.txt | tee intermediate.txt | grep pattern

# Error handling in pipelines
command1 2>&1 | command2  # Include stderr in pipe
```

## Pipe vs Other IPC Methods

- **Pipes**: Simple, unidirectional, process-local
- **Named Pipes (FIFOs)**: Persistent, can connect unrelated processes
- **Sockets**: Network communication, bidirectional
- **Shared Memory**: Fastest, but requires synchronization
- **Message Queues**: Structured messages, persistent

## Experiment Suggestions

1. Create a three-process pipeline (A | B | C)
2. Implement a simple shell that supports pipes
3. Build a producer-consumer system with pipes
4. Try sending binary data through pipes
5. Experiment with large data and observe buffering behavior
6. Create a pipe-based calculator (send expressions, receive results)
