# Thread Basics

## Definition and Concept

A thread is a lightweight, schedulable unit of execution within a process. Threads are sometimes referred to as "lightweight processes" because they require fewer resources to create and maintain than full processes.

Each thread represents an independent flow of control within a process and has its own:

- Thread ID
- Program counter
- Register set
- Stack

However, all threads within a process share:

- Code section
- Data section
- Other operating system resources such as open files and signals

## Process vs. Thread

Understanding the differences between processes and threads is fundamental to concurrent programming:

### Process Characteristics

- Independent execution unit with its own memory space
- Contains one or more threads
- Processes don't share memory with other processes by default
- Communication between processes requires IPC (Inter-Process Communication)
- Creation and context switching are relatively expensive operations
- Greater isolation and protection between processes

### Thread Characteristics

- Lightweight execution unit within a process
- Shares memory and resources with other threads in the same process
- Direct communication through shared memory
- Creation and context switching are less expensive
- Less isolation, but higher efficiency for cooperative tasks

```
┌───────────────────────────────────────┐
│               Process                 │
│                                       │
│  ┌─────────┐   ┌─────────┐            │
│  │ Thread 1│   │ Thread 2│   ...      │
│  └─────────┘   └─────────┘            │
│                                       │
│  ┌───────────────────────────────────┐│
│  │           Code Section            ││
│  └───────────────────────────────────┘│
│                                       │
│  ┌───────────────────────────────────┐│
│  │          Data Section             ││
│  └───────────────────────────────────┘│
│                                       │
│  ┌───────────────────────────────────┐│
│  │             Heap                  ││
│  └───────────────────────────────────┘│
│                                       │
│  ┌─────────┐   ┌─────────┐            │
│  │ Stack 1 │   │ Stack 2 │   ...      │
│  └─────────┘   └─────────┘            │
│                                       │
└───────────────────────────────────────┘
```

## Thread Components

Each thread maintains its own:

### 1. Thread ID

- Unique identifier within the process
- Used to refer to specific threads in the system

### 2. Program Counter (PC)

- Points to the current instruction being executed
- Allows each thread to execute different code paths

### 3. Register Set

- CPU registers that maintain the execution state
- Saved and restored during context switching

### 4. Stack

- Private stack for local variables and function calls
- Each thread has its own stack space
- Function parameters, return addresses, and local variables

### Thread Control Block (TCB)

Similar to Process Control Block (PCB), the Thread Control Block is a data structure that stores thread-specific information:

- Thread ID
- Register values
- Stack pointer
- Priority
- Thread state
- CPU scheduling information
- Thread-specific data (thread local storage)

## Context Switching: Threads vs. Processes

Context switching is the mechanism of saving the current execution state to resume it later and loading a different execution state.

### Process Context Switch

When switching between processes:

1. Save all CPU registers of Process A to its PCB
2. Update the memory management structures (page tables)
3. Reload CPU registers of Process B from its PCB
4. Switch address spaces (potentially invalidating caches and TLB)

### Thread Context Switch

When switching between threads within the same process:

1. Save CPU registers of Thread A to its TCB
2. Reload CPU registers of Thread B from its TCB
3. No need to switch address spaces or memory management structures

### Performance Comparison

Thread context switches are significantly faster than process context switches because:

- No need to change memory mappings
- Cache and TLB entries remain valid
- No need to flush translation lookaside buffer (TLB)

On a typical system:
- Process context switch: ~1000-10000 nanoseconds
- Thread context switch: ~100-1000 nanoseconds (roughly 10x faster)

## Thread States and Life Cycle

Like processes, threads have various states throughout their life cycle:

### 1. New

- Thread has been created but not yet started
- Resources are allocated
- Thread is not yet scheduled for execution

### 2. Ready

- Thread is ready to run but waiting for CPU time
- Placed in the scheduler's ready queue

### 3. Running

- Thread is currently executing on a CPU core
- Only one thread per core can be in this state

### 4. Blocked/Waiting

- Thread is waiting for some event (I/O completion, mutex acquisition)
- Cannot continue execution until the event occurs

### 5. Terminated

- Thread has completed execution or been terminated
- Resources may not be reclaimed until joined by another thread

```
                  ┌─────────┐
                  │   New   │
                  └────┬────┘
                       │ start()
                       ▼
                  ┌─────────┐          ┌─────────┐
                  │  Ready  │◄────────┐│ Blocked │
                  └────┬────┘         │└────┬────┘
                       │              │     │
                       │ scheduled    │     │ event occurred
                       ▼              │     │
                  ┌─────────┐         │     │
                  │ Running │─────────┘     │
                  └────┬────┘ wait for event│
                       │                    │
                       │ completed          │
                       ▼                    │
                  ┌─────────┐               │
                  │Terminated               │
                  └─────────┘               │
```

## Thread Stacks

Each thread in a process has its own stack for:

1. **Function Call Management**:
   - Return addresses
   - Function parameters
   - Call frame linkage

2. **Local Variable Storage**:
   - Variables declared within functions
   - Temporary data

3. **Exception Handling**:
   - Try-catch blocks
   - Exception unwinding information

Thread stacks are independent to ensure that function calls in one thread don't interfere with those in another thread.

### Stack Size Considerations

- Default thread stack size varies by system (typically 1-8 MB)
- Too small: Risk of stack overflow with deep recursion or large local variables
- Too large: Waste of virtual address space
- Some threading APIs allow customizing stack size when creating threads

## Benefits of Threads

Threads provide several advantages in program design and performance:

### 1. Resource Sharing

- Threads share the same address space
- No need for explicit IPC mechanisms
- Direct access to shared data

### 2. Economy

- Thread creation is faster than process creation
- Less overhead in thread management
- Lower memory consumption per thread

### 3. Responsiveness

- Application can remain responsive during lengthy operations
- Interactive applications can handle user input while processing

### 4. Scalability

- Can take advantage of multiple processors or cores
- Parallel execution of tasks

### 5. Program Structure

- More natural decomposition of some problems
- Simpler program structure for certain tasks (like server applications)

## Challenges with Threads

Despite their advantages, threads introduce some challenges:

### 1. Synchronization Issues

- Race conditions when multiple threads access shared data
- Need for coordination mechanisms (mutexes, semaphores)

### 2. Debugging Difficulty

- Non-deterministic behavior
- Hard to reproduce bugs
- Complex interactions between threads

### 3. Design Complexity

- Requires careful planning to avoid deadlocks and starvation
- Thread-safe programming practices required

### 4. Overhead

- Thread creation and management has its own overhead
- Too many threads can degrade performance (context switching)

## Concurrency vs. Parallelism

These terms are often confused but represent different concepts:

### Concurrency

- Multiple tasks making progress over overlapping time periods
- Can occur on a single processor through time-slicing
- About dealing with multiple things at once (structure)
- Example: A single-core CPU running multiple threads

### Parallelism

- Multiple tasks executing simultaneously
- Requires multiple execution units (cores/processors)
- About doing multiple things at once (execution)
- Example: Matrix multiplication distributed across multiple CPU cores

```
Concurrency (Single core with time slicing):
Thread A: ──█───────█───────█───────█──
Thread B: ────█───────█───────█───────

Parallelism (Multiple cores):
Thread A: ──████████████████████████──
Thread B: ──████████████████████████──
```

## Real-World Thread Use Cases

Threads are extensively used in various applications:

### 1. Web Servers

- Dedicated thread for each client connection
- Handles requests concurrently
- Example: Apache HTTP Server's thread-per-connection model

### 2. Web Browsers

- Separate threads for:
  - Rendering the UI
  - Processing JavaScript
  - Network communication
  - Plugin execution

### 3. Video Games

- Rendering thread
- Physics calculation thread
- AI thread
- Input processing thread

### 4. Multimedia Applications

- User interface thread
- Media decoding thread
- Audio processing thread
- Real-time effects thread

### 5. Database Systems

- Connection handler threads
- Query execution threads
- Background maintenance threads

### 6. Machine Learning Frameworks

- Data loading and preprocessing threads
- Computation threads
- Result aggregation threads