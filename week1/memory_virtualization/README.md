# Memory Virtualization

## Overview

This program demonstrates memory virtualization by showing how each process has its own virtual address space. Multiple instances will show the same virtual addresses but they refer to different physical memory locations.

## Key Concepts

- **Virtual Memory**: Each process has its own virtual address space
- **Address Translation**: Virtual addresses are mapped to physical addresses by the OS
- **Memory Isolation**: Processes cannot access each other's memory
- **Process ID (PID)**: Unique identifier for each running process

## How to Compile and Run

### Compile

```bash
gcc -o mem mem.c
```

### Run Single Instance

```bash
./mem
```

### Run Multiple Instances (demonstrates virtualization)

```bash
./mem & ./mem & ./mem &
```

### To stop all background processes

```bash
killall mem
```

## Expected Output

Each instance will show:

- The same virtual address (e.g., 0x559abc123000)
- Different process IDs
- Independent counter values

Example output:

```bash
(1234) address of p: 0x559abc123000
(1234) value of p: 1 and addr pointed to by p: 0x559abc123000
(1235) address of p: 0x559abc123000
(1235) value of p: 1 and addr pointed to by p: 0x559abc123000
```

## Learning Points

1. Same virtual addresses in different processes point to different physical memory
2. Each process has its own private memory space
3. The OS provides the illusion of a large, private address space to each process
4. Memory virtualization enables process isolation and security

## Additional Commands to Explore Memory

```bash
# View memory layout of a process
cat /proc/PID/maps

# View memory usage
ps aux | grep mem

# Monitor memory usage in real-time
top -p PID
```
