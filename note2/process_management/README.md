# Process Management

## Process Creation

When a new process is created in an operating system, several steps take place to set up the execution environment properly.

### Process Creation Steps

1. **Memory Allocation**: The operating system allocates memory for the process's address space.

2. **Loading Program Code**:
   - Program code is loaded from secondary storage (disk) into the allocated memory.
   - Two main loading approaches:
     - **Eager Loading**: Entire program is loaded at once (traditional approach).
     - **Lazy Loading**: Only necessary parts are loaded as needed (modern approach).

3. **Stack Initialization**:
   - Stack area is allocated.
   - Command line arguments (argc, argv) are placed on the stack.
   - Environment variables are also placed on the stack.

4. **Heap Initialization**:
   - Heap area is initialized for dynamic memory allocation.
   - Initially small, expands as needed.

5. **I/O Initialization**:
   - Standard file descriptors are set up (stdin, stdout, stderr).
   - Other requested files are opened.

6. **PCB Creation**:
   - Process Control Block is created and initialized.
   - Process is assigned a unique Process ID (PID).

7. **Start Execution**:
   - CPU registers are initialized.
   - Program counter is set to the program's entry point (typically the main() function).
   - Process is placed in the ready queue.

## Process Creation in UNIX-Like Systems

In UNIX and Linux systems, processes are created using two primary system calls: `fork()` and `exec()`.

### The `fork()` System Call

The `fork()` system call creates a new process by duplicating the calling process:

```c
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process, PID: %d\n", getpid());
    } else {
        // Parent process
        printf("Parent process, PID: %d, Child PID: %d\n", getpid(), pid);
    }
    
    return 0;
}
```

Key aspects of `fork()`:

- Creates an exact duplicate of the calling process.
- Child receives a copy of the parent's data, heap, and stack segments.
- Child shares the text segment with the parent (read-only code).
- Returns different values in the parent and child:
  - In the parent: Returns the PID of the newly created child.
  - In the child: Returns 0.
  - On failure: Returns -1.

### The `exec()` Family of System Calls

The `exec()` system calls replace the current process image with a new one:

```c
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process before exec, PID: %d\n", getpid());
        
        // Replace child process with a new program
        execl("/bin/ls", "ls", "-l", NULL);
        
        // This line is reached only if execl fails
        fprintf(stderr, "Exec failed\n");
        return 1;
    } else {
        // Parent process
        printf("Parent process, PID: %d, Child PID: %d\n", getpid(), pid);
    }
    
    return 0;
}
```

Common `exec()` variants:

- `execl()`: List arguments individually (NULL terminated).
- `execv()`: Take arguments as an array.
- `execle()`: Like `execl()`, but specify environment.
- `execve()`: Like `execv()`, but specify environment.
- `execlp()`, `execvp()`: Search for executable in PATH.

When an `exec()` call is successful, the current process is completely replaced, and execution starts from the new program's main function.

### Fork-Exec Pattern

The combination of `fork()` and `exec()` is a common pattern in UNIX systems:

1. `fork()` creates a new process (child).
2. The child process calls `exec()` to run a new program.
3. The parent can continue execution or wait for the child.

This pattern is used when starting new programs, such as when a shell executes a command.

## Process Termination

A process can terminate in several ways:

### Normal Termination

When a process completes its execution normally:

```c
#include <stdlib.h>

int main() {
    // Process work
    
    // Normal exit with status 0 (success)
    exit(0);
    
    // Or implicitly with return
    return 0;
}
```

The `exit()` function or returning from `main()` causes:
- Closing all open files
- Releasing allocated memory
- Sending termination status to parent
- Turning the process into a "zombie" until parent acknowledges

### Abnormal Termination

A process can be terminated abnormally due to:

1. **Fatal Error**:
   - Segmentation fault
   - Illegal instruction
   - Divide by zero

2. **Explicit Termination**:
   - `abort()` function call
   - Receiving a signal (e.g., SIGTERM, SIGKILL)

3. **Parent Termination**:
   - If parent terminates, children may be terminated or adopted by init process (PID 1)

## Process Scheduling

The process scheduler is responsible for determining which process runs next on the CPU.

### Types of Processes Based on Behavior

1. **CPU-bound processes**:
   - Spend most of their time using the CPU
   - Long computing tasks with minimal I/O
   - Examples: scientific simulations, video encoding

2. **I/O-bound processes**:
   - Spend most of their time waiting for I/O operations
   - Brief CPU bursts followed by I/O requests
   - Examples: text editors, database queries

### Scheduling Queues

The operating system maintains several queues to manage processes:

1. **Job Queue**:
   - Contains all processes in the system
   - Managed by long-term scheduler (admission scheduler)

2. **Ready Queue**:
   - Contains processes ready to execute
   - Managed by short-term scheduler (CPU scheduler)

3. **Device Queues**:
   - One queue per I/O device
   - Contains processes waiting for specific I/O operations

```
                ┌─────────┐            ┌─────────┐
                │   Job   │            │  Ready  │
                │  Queue  │───────────►│  Queue  │◄─────────┐
                └─────────┘            └────┬────┘          │
                                            │               │
                                            │               │
                                            ▼               │
                                       ┌─────────┐          │
                                       │   CPU   │          │
                                       └────┬────┘          │
                                            │               │
                                            │               │
                                            ▼               │
                                  ┌──────────────────┐      │
                                  │ I/O or Event Wait│      │
                                  └────────┬─────────┘      │
                                           │                │
                                           │                │
                                           ▼                │
                                       ┌────────┐           │
                                       │ Device │           │
                                       │ Queues │───────────┘
                                       └────────┘
```

### Schedulers

An operating system employs different types of schedulers:

1. **Long-term Scheduler** (Job Scheduler):
   - Controls the degree of multiprogramming
   - Decides which programs to admit to the system
   - Infrequently run (seconds, minutes)

2. **Short-term Scheduler** (CPU Scheduler):
   - Selects which process should be executed next
   - Allocates CPU to the selected process
   - Very frequently run (milliseconds)

3. **Medium-term Scheduler**:
   - Temporarily removes processes from memory to reduce system load (swapping)
   - Reintroduces them later when conditions improve
   - Run occasionally (seconds to minutes)

## Context Switching

Context switching is the process of saving the state of a currently running process and restoring the state of a different process for execution.

### Context Switch Process

1. **Save the context** of the currently running process:
   - Save CPU registers
   - Update PCB
   - Change process state

2. **Select a new process** to run from the ready queue.

3. **Restore the context** of the selected process:
   - Load CPU registers
   - Update memory management information
   - Change process state to running

```
   Process A               Process B
     running                waiting
        │                      │
┌───────▼────────┐             │
│  Save context  │             │
│  of Process A  │             │
└───────┬────────┘             │
        │                      │
┌───────▼────────┐             │
│ Select Process │             │
│      to run    │             │
└───────┬────────┘             │
        │                      │
┌───────▼────────┐             │
│ Restore context│             │
│  of Process B  │             │
└───────┬────────┘             │
        │                      │
        └──────────────────────┘
                 │
                 ▼
              running
```

### Context Switch Overhead

Context switching incurs significant overhead:

1. **Direct costs**:
   - CPU time spent saving and loading registers
   - Updating various data structures (PCB, etc.)

2. **Indirect costs**:
   - Cold cache: Loss of cache locality
   - TLB flushes: Memory access slowdown
   - Pipeline stalls: CPU execution efficiency reduced

The overhead of context switching is one reason why threads are sometimes preferred over processes for concurrent programming, as thread context switches are less expensive.

## Process Communication

Processes need mechanisms to communicate with each other and synchronize their actions. Inter-Process Communication (IPC) provides these mechanisms.

### IPC Methods

1. **Shared Memory**:
   - Fastest IPC method
   - Region of memory shared between processes
   - Requires synchronization mechanisms

2. **Message Passing**:
   - Processes exchange messages
   - Can be implemented as pipes, message queues, or sockets
   - Operating system manages communication

3. **Pipes**:
   - Unidirectional communication channel
   - Common for parent-child process communication
   - Example: command line pipes (`cmd1 | cmd2`)

4. **Signals**:
   - Software interrupts sent to a process
   - Limited information can be conveyed
   - Used for event notification (e.g., SIGTERM, SIGKILL)

5. **Sockets**:
   - Communication endpoints
   - Can communicate between processes on different machines
   - Used extensively in network programming

6. **Message Queues**:
   - Linked list of messages stored within the kernel
   - Each message has a type for message selection

7. **Semaphores**:
   - Synchronization primitives
   - Used to control access to shared resources

### Synchronization Challenges

When processes communicate, especially via shared memory, several challenges arise:

1. **Race Conditions**:
   - Multiple processes accessing shared data concurrently
   - Final result depends on the timing of executions

2. **Critical Sections**:
   - Portions of code that access shared resources
   - Need mutual exclusion

3. **Deadlocks**:
   - Two or more processes waiting for resources held by each other
   - System becomes stuck

These challenges are addressed through various synchronization mechanisms like mutexes, semaphores, and monitors.