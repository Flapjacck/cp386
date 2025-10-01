# CP386 Operating Systems Course Materials - Glossary

This repository contains practical examples and demonstrations for CP386 Operating Systems course concepts. Each topic includes working C code examples with comprehensive documentation.

## Topic Glossary

### Note 5: Scheduling

**[CPU Scheduling](note5/cpu_scheduling/README.md)**

- **Concept**: How OS decides which processes to run when and for how long
- **Key Algorithms**: FCFS, SJF, STCF, Round Robin
- **Key APIs**: Process scheduling, time slicing, waiting queues
- **Demonstration**: Comparing scheduling algorithms with different workloads
- **Learn**: Turnaround time, response time, quantum size trade-offs, convoy effect

**[Multi-Level Feedback Queue](note5/multilevel_feedback/README.md)**

- **Concept**: Adaptive scheduling that learns from process behavior
- **Key Features**: Dynamic priority adjustment, prevention of starvation
- **Key APIs**: Queue management, priority boosting, time accounting
- **Demonstration**: Interactive vs CPU-bound process handling
- **Learn**: How OS distinguishes between interactive and CPU-bound jobs

### Note 7: Multiprocessor and Synchronization

**[Multi-CPU Scheduling](note7/multi_cpu_scheduling/README.md)**

- **Concept**: Scheduling in multicore processors, balancing loads across CPUs
- **Key Approaches**: Single-Queue vs Multi-Queue scheduling, work stealing
- **Key APIs**: CPU affinity, load balancing
- **Demonstration**: Cache effects on multiprocessor performance
- **Learn**: Cache coherence, cache affinity, load balancing techniques

**[Synchronization and Locks](note7/synchronization_locks/README.md)**

- **Concept**: Protecting shared resources in concurrent environments
- **Key Mechanisms**: Mutex locks, spin locks, ticket locks
- **Key APIs**: `pthread_mutex_t`, atomic operations
- **Demonstration**: Race conditions and their solutions
- **Learn**: Race conditions, critical sections, atomic operations, deadlocks

### Note 1: Introduction - Virtualization Concepts

**[CPU Virtualization](note1/cpu_virtualization/README.md)**

- **Concept**: How the OS creates the illusion that each process has its own dedicated CPU
- **Key APIs**: `Spin()`, `printf()`, process scheduling
- **Demonstration**: Multiple processes printing interleaved output showing time-sharing
- **Learn**: Process scheduling, context switching, time-sharing mechanisms

**[Memory Virtualization](note1/memory_virtualization/README.md)**

- **Concept**: Each process has its own virtual address space isolated from others
- **Key APIs**: `malloc()`, `getpid()`, virtual memory management
- **Demonstration**: Same virtual addresses in different processes point to different physical memory
- **Learn**: Virtual memory, address translation, process isolation, memory protection

**[Threads and Concurrency](note1/threads/README.md)**

- **Concept**: Lightweight processes sharing memory within a single process
- **Key APIs**: `pthread_create()`, `pthread_join()`, shared memory access
- **Demonstration**: Race conditions and lost updates in concurrent counter increments
- **Learn**: Thread synchronization, race conditions, atomic operations, critical sections

### Note 3: Process API

**[Process Creation](note3/process_creation/README.md)**

- **Concept**: Creating new processes and controlling their execution order
- **Key APIs**: `fork()`, `wait()`, `getpid()`, process hierarchy
- **Demonstration**: Parent-child process relationships and synchronization
- **Learn**: Process creation, process IDs, parent-child relationships, process synchronization

**[Process Execution](note3/process_execution/README.md)**

- **Concept**: Replacing process image with new programs (how shells launch commands)
- **Key APIs**: `execvp()`, `exec()` family, program loading, argument passing
- **Demonstration**: Child process becoming `wc` command, shell-like behavior
- **Learn**: Process image replacement, program execution, shell internals, system calls

**[I/O Redirection](note3/io_redirection/README.md)**

- **Concept**: Redirecting standard input/output streams between processes and files
- **Key APIs**: `close()`, `open()`, `dup()`, file descriptor manipulation
- **Demonstration**: Redirecting command output to files, stdin/stdout manipulation
- **Learn**: File descriptors, stream redirection, shell I/O operations, Unix I/O model

**[Pipes and IPC](note3/pipes/README.md)**

- **Concept**: Inter-process communication using anonymous pipes
- **Key APIs**: `pipe()`, `dup2()`, process communication, data streaming
- **Demonstration**: Connecting process output to input (shell pipelines like `echo | wc`)
- **Learn**: Inter-process communication, pipe mechanics, Unix pipeline philosophy
