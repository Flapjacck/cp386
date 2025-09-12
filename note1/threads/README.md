# Threads and Concurrency

## Overview

This program demonstrates the challenges of concurrent programming by showing how two threads incrementing a shared counter can lead to race conditions and lost updates when the number of iterations is large.

## Key Concepts

- **Threads**: Lightweight processes that share memory within a single process
- **Race Conditions**: When multiple threads access shared data simultaneously
- **Atomic Operations**: Operations that complete without interruption
- **Shared Memory**: Memory accessible by multiple threads
- **Critical Sections**: Code that accesses shared resources

## How to Compile and Run

### Compile

```bash
gcc -o thread thread.c -lpthread
```

### Run with Small Loop Count (usually works correctly)

```bash
./thread 1000
```

### Run with Large Loop Count (demonstrates race conditions)

```bash
./thread 100000
./thread 1000000
```

### Multiple Runs to See Variation

```bash
for i in {1..5}; do ./thread 100000; done
```

## Expected Output

### Small Loop Count

```bash
Initial value : 0
Final value : 2000
Expected value: 2000
```

### Large Loop Count (shows race condition)

```bash
Initial value : 0
Final value : 1823445
Expected value: 2000000
```

## Why This Happens

The increment operation `counter++` is not atomic. It actually involves three steps:

1. Load the value of counter from memory
2. Increment the value
3. Store the value back to memory

When two threads do this simultaneously, they can interfere with each other, causing lost updates.

## Learning Points

1. Shared memory between threads can lead to race conditions
2. Simple operations like `++` are not atomic at the machine level
3. As the number of operations increases, the likelihood of race conditions increases
4. Proper synchronization (locks, atomic operations) is needed for thread safety

## Additional Threading Commands

```bash
# View threads of a running process
ps -eLf | grep thread

# Monitor CPU usage by threads
top -H

# Compile with debugging symbols
gcc -g -o thread thread.c -lpthread

# Run with thread-safe debugging
valgrind --tool=helgrind ./thread 10000
```

## Experiment Suggestions

1. Try different loop counts and observe when race conditions start appearing
2. Run the same command multiple times and notice the variation in results
3. Compare results on systems with different numbers of CPU cores
