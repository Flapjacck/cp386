# CPU Virtualization

## Overview

This program demonstrates CPU virtualization by showing how the operating system can interleave the execution of multiple processes, each running an infinite loop. When multiple instances are run simultaneously, you'll see their output interleaved, demonstrating time-sharing.

## Key Concepts

- **CPU Virtualization**: The OS creates the illusion that each process has its own dedicated CPU
- **Time-sharing**: Multiple processes share CPU time through rapid context switching
- **Process Scheduling**: The OS scheduler determines which process runs when

## How to Compile and Run

### Compile

```bash
gcc -o cpu cpu.c
```

### Run Single Instance

```bash
./cpu A
```

### Run Multiple Instances (demonstrates virtualization)

```bash
./cpu A & ./cpu B & ./cpu C & ./cpu D &
```

### To stop all background processes

```bash
killall cpu
```

## Expected Output

When running multiple instances, you should see interleaved output like:

```bash
A
B
C
D
A
B
C
D
...
```

The exact order will vary due to the non-deterministic nature of process scheduling.

## Learning Points

1. Each process thinks it has the CPU to itself
2. The OS rapidly switches between processes (context switching)
3. From the user's perspective, processes appear to run simultaneously
4. The `Spin(1)` function simulates work being done by the process
