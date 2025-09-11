# Process Creation with fork() and wait()

## Overview

These programs demonstrate the fundamental process creation system calls in Unix-like systems: `fork()` and `wait()`. They show how processes are created, how parent and child processes differ, and how to control execution order.

## Key Concepts

- **fork()**: Creates a new process by duplicating the current process
- **Process ID (PID)**: Unique identifier for each process
- **Parent Process**: The process that calls fork()
- **Child Process**: The new process created by fork()
- **wait()**: Makes parent process wait for child to complete
- **Return Values**: fork() returns different values to parent and child

## Programs

### p1.c - Basic fork()

Demonstrates basic process creation without synchronization.

### p2.c - fork() with wait()

Shows how to make the parent wait for the child to finish.

## How to Compile and Run

### Compile

```bash
gcc -o p1 p1.c
gcc -o p2 p2.c
```

### Run p1 (nondeterministic order)

```bash
./p1
```

### Run p2 (deterministic order)

```bash
./p2
```

### Run multiple times to see variation

```bash
for i in {1..5}; do ./p1; echo "---"; done
```

## Expected Output

### p1.c Output (order may vary)

```bash
hello world (pid:1234)
hello, I am parent of 1235 (pid:1234)
hello, I am child (pid:1235)
```

OR

```bash
hello world (pid:1234)
hello, I am child (pid:1235)
hello, I am parent of 1235 (pid:1234)
```

### p2.c Output (consistent order)

```bash
hello world (pid:1234)
hello, I am child (pid:1235)
hello, I am parent of 1235 (wc:1235) (pid:1234)
```

## Understanding fork() Return Values

- **In parent process**: fork() returns the child's PID (positive number)
- **In child process**: fork() returns 0
- **On error**: fork() returns -1

## Learning Points

1. fork() creates an exact copy of the current process
2. Both parent and child continue execution from the point of fork()
3. Without wait(), execution order is nondeterministic
4. wait() ensures parent waits for child completion
5. Each process has its own unique PID

## Additional Process Commands

```bash
# View running processes
ps aux

# View process tree
pstree

# Monitor process creation
ps -ef --forest

# View process information in real-time
top

# Kill a process by PID
kill PID
```

## Experiment Suggestions

1. Run p1 multiple times and observe different output orders
2. Add sleep() calls in different places to see timing effects
3. Create multiple children with multiple fork() calls
