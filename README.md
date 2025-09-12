# CP386 Operating Systems Course Materials - Glossary

This repository contains practical examples and demonstrations for CP386 Operating Systems course concepts. Each topic includes working C code examples with comprehensive documentation.

## Topic Glossary

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
