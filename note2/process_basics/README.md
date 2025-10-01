# Process Basics

## Definition and Concept

A process is an instance of a program in execution. It is the fundamental unit of execution in an operating system. When a program is loaded from secondary storage (such as a hard drive) into main memory and executed, it becomes a process.

Examples of processes:
- Running the GCC compiler: `gcc file.c`
- Opening a web browser: `firefox`
- Using a text editor: `emacs`

Each process is an independent entity with its own:

1. **Program Code (Text)**: Instructions to be executed
2. **Execution Context**: Current state of CPU registers and program counter
3. **Memory Layout**: Organization of the process in memory
4. **Resources**: Files, I/O devices, etc.

## Process Memory Layout

A process's memory layout is typically divided into several segments:

### 1. Text Segment (Code Segment)

- Contains executable instructions
- Read-only to prevent accidental modification
- Often shared among processes running the same program
- Fixed size determined at compile time

```
High memory addresses
+------------------+
|                  |
|      Stack       | → Grows downward
|        ↓         |
+------------------+
|                  |
|        ↑         |
|       Heap       | → Grows upward
|                  |
+------------------+
|   Uninitialized  |
|  Data (BSS seg)  |
+------------------+
|    Initialized   |
|      Data        |
+------------------+
|                  |
|      Text        |
|                  |
+------------------+
Low memory addresses
```

### 2. Data Segment

Split into two parts:

**Initialized Data Segment**:
- Contains global and static variables that are explicitly initialized
- Example: `static int i = 10;` or `int global = 5;`
- Loaded from the executable file

**Uninitialized Data Segment (BSS)**:
- Contains uninitialized global and static variables
- Initialized to zero by the kernel before program starts
- Example: `static int j;` or `int global_array[100];`
- Not stored in executable file (saves space)

### 3. Heap

- Region for dynamic memory allocation
- Managed through `malloc()`, `free()` in C
- Grows upward (toward higher memory addresses)
- Not automatically managed (manual memory management)
- Can lead to memory leaks or fragmentation if not properly managed

### 4. Stack

- Contains function call frames, local variables, and return addresses
- LIFO (Last In, First Out) structure
- Grows downward (toward lower memory addresses)
- Automatically managed
- Limited in size (can cause stack overflow)

For each function call, a new stack frame is created containing:
- Local variables
- Function parameters
- Return address
- Saved registers

## Process Address Space

Each process has its own virtual address space, which the OS maps to physical memory. This provides:

1. **Process Isolation**: One process can't access another's memory
2. **Memory Protection**: OS can protect regions of memory
3. **Memory Efficiency**: Virtual memory allows processes to use more memory than physically available

## Process Control Block (PCB)

The Process Control Block (PCB) is a data structure that the operating system uses to store all information about a process. It is the manifestation of a process in an operating system.

### Components of PCB

1. **Process Identification**
   - Process ID (PID): Unique identifier
   - Parent Process ID (PPID)
   - User ID: Owner of the process

2. **Process State**
   - New: Process is being created
   - Ready: Process is waiting to be assigned to a processor
   - Running: Instructions are being executed
   - Waiting/Blocked: Process is waiting for some event (I/O completion, signal reception)
   - Terminated: Process has finished execution

3. **Program Counter**
   - Address of the next instruction to be executed

4. **CPU Registers**
   - General-purpose registers
   - Stack pointer
   - Program status word (PSW)
   - Condition codes

5. **Memory Management Information**
   - Base and limit registers
   - Page tables or segment tables
   - Memory allocation information

6. **I/O Status Information**
   - List of open files
   - I/O device allocations
   - I/O requests pending

7. **Accounting Information**
   - CPU time used
   - Time limits
   - Account numbers

8. **Scheduling Information**
   - Process priority
   - Pointers to scheduling queues
   - Other scheduling parameters

The PCB is created when a process is created and is deleted when the process terminates. It's updated throughout the process's lifetime.

```
┌───────────────────┐
│   Process ID      │
├───────────────────┤
│   Process State   │
├───────────────────┤
│  Program Counter  │
├───────────────────┤
│   CPU Registers   │
├───────────────────┤
│ Memory Management │
│   Information     │
├───────────────────┤
│   I/O Status      │
│   Information     │
├───────────────────┤
│   Accounting      │
│   Information     │
└───────────────────┘
```

## Process States and Transitions

A process goes through various states during its lifetime:

### 1. New
- Process is being created
- OS allocates resources
- PCB is initialized

### 2. Ready
- Process is waiting to be assigned to a processor
- All resources available except the CPU
- Ready processes are in the ready queue

### 3. Running
- Process is being executed by the CPU
- Instructions are being carried out
- Only one process per CPU can be in this state at a time

### 4. Waiting/Blocked
- Process is waiting for an event (I/O completion, resource availability)
- Cannot execute until event occurs
- Moved to ready state when event completes

### 5. Terminated
- Process has finished execution
- OS cleans up resources
- PCB is deleted

### State Transitions

- **New → Ready**: When process is loaded into main memory
- **Ready → Running**: When scheduler selects process for execution
- **Running → Ready**: When scheduler decides to replace current process (time quantum expires)
- **Running → Waiting**: When process requests I/O or waits for an event
- **Waiting → Ready**: When requested I/O or event completes
- **Running → Terminated**: When process completes execution or is aborted

```
                  ┌─────────┐
                  │   New   │
                  └────┬────┘
                       │
                       ▼
                  ┌─────────┐          ┌─────────┐
                  │  Ready  │◄────────┐│ Waiting │
                  └────┬────┘         │└─────────┘
                       │              │     ▲
                       ▼              │     │
                  ┌─────────┐         │     │
                  │ Running │─────────┘     │
                  └────┬────┘               │
                       │                    │
                       ▼                    │
                  ┌─────────┐               │
                  │Terminated               │
                  └─────────┘               │
                       ┌──────────────────────┘
```

## Process vs. Program

It's important to understand the distinction between a process and a program:

| Program | Process |
|---------|---------|
| Passive entity | Active entity |
| Binary file on disk | Program in execution |
| Static | Dynamic |
| No resources allocated | Resources allocated |
| No execution state | Has execution state |
| Can exist indefinitely | Exists temporarily |

Multiple processes can be associated with the same program. For example, opening multiple tabs in a web browser may create a new process for each tab, all running the same browser program.