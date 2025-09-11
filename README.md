# CP386 Operating Systems Course Materials - Glossary

This repository contains practical examples and demonstrations for CP386 Operating Systems course concepts. Each topic includes working C code examples with comprehensive documentation.

## Topic Glossary

### Week 1: Introduction - Virtualization Concepts

**[CPU Virtualization](week1/cpu_virtualization/README.md)**
- **Concept**: How the OS creates the illusion that each process has its own dedicated CPU
- **Key APIs**: `Spin()`, `printf()`, process scheduling
- **Demonstration**: Multiple processes printing interleaved output showing time-sharing
- **Learn**: Process scheduling, context switching, time-sharing mechanisms

**[Memory Virtualization](week1/memory_virtualization/README.md)**
- **Concept**: Each process has its own virtual address space isolated from others
- **Key APIs**: `malloc()`, `getpid()`, virtual memory management
- **Demonstration**: Same virtual addresses in different processes point to different physical memory
- **Learn**: Virtual memory, address translation, process isolation, memory protection

**[Threads and Concurrency](week1/threads/README.md)**
- **Concept**: Lightweight processes sharing memory within a single process
- **Key APIs**: `pthread_create()`, `pthread_join()`, shared memory access
- **Demonstration**: Race conditions and lost updates in concurrent counter increments
- **Learn**: Thread synchronization, race conditions, atomic operations, critical sections

### Week 3: Process API

**[Process Creation](week3/process_creation/README.md)**
- **Concept**: Creating new processes and controlling their execution order
- **Key APIs**: `fork()`, `wait()`, `getpid()`, process hierarchy
- **Demonstration**: Parent-child process relationships and synchronization
- **Learn**: Process creation, process IDs, parent-child relationships, process synchronization

**[Process Execution](week3/process_execution/README.md)**
- **Concept**: Replacing process image with new programs (how shells launch commands)
- **Key APIs**: `execvp()`, `exec()` family, program loading, argument passing
- **Demonstration**: Child process becoming `wc` command, shell-like behavior
- **Learn**: Process image replacement, program execution, shell internals, system calls

**[I/O Redirection](week3/io_redirection/README.md)**
- **Concept**: Redirecting standard input/output streams between processes and files
- **Key APIs**: `close()`, `open()`, `dup()`, file descriptor manipulation
- **Demonstration**: Redirecting command output to files, stdin/stdout manipulation
- **Learn**: File descriptors, stream redirection, shell I/O operations, Unix I/O model

**[Pipes and IPC](week3/pipes/README.md)**
- **Concept**: Inter-process communication using anonymous pipes
- **Key APIs**: `pipe()`, `dup2()`, process communication, data streaming
- **Demonstration**: Connecting process output to input (shell pipelines like `echo | wc`)
- **Learn**: Inter-process communication, pipe mechanics, Unix pipeline philosophy

## Quick Start

### Prerequisites

```bash
# Install development tools (Ubuntu/Debian)
sudo apt update
sudo apt install build-essential

# Or on Red Hat/CentOS/Fedora
sudo yum groupinstall "Development Tools"
# or
sudo dnf groupinstall "Development Tools"
```

### Build Everything

```bash
make all
```

### Clean Build Files

```bash
make clean
```

### Build Specific Week

```bash
make week1
make week3
```

## Learning Path

1. **Start with Week 1** to understand virtualization concepts
   - Begin with `cpu_virtualization` to see time-sharing in action
   - Move to `memory_virtualization` to understand address spaces
   - Explore `threads` to see concurrency challenges

2. **Continue with Week 3** to learn process management
   - Study `process_creation` for basic process operations
   - Learn `process_execution` for program launching
   - Understand `io_redirection` for stream manipulation
   - Master `pipes` for inter-process communication

## Key Learning Outcomes

### Week 1

- Understanding how OS virtualizes CPU and memory
- Seeing time-sharing and scheduling in action
- Learning about race conditions in concurrent programs

### Week 3

- Mastering process creation and management
- Understanding how shells work internally
- Learning Unix I/O redirection mechanisms
- Building inter-process communication systems

## Usage Tips

1. **Read the README.md** in each topic folder before running code
2. **Compile and run** examples as suggested in each README
3. **Experiment** with different parameters and scenarios
4. **Compare** program behavior with equivalent shell commands

## Course Integration

These examples directly correspond to lecture slides and demonstrate:

- Fundamental OS concepts through working code
- System call usage and behavior
- Real-world applications of theoretical concepts
- Hands-on experience with Unix/Linux system programming

## Additional Resources

- Man pages: `man fork`, `man exec`, `man pipe`, etc.
- System call tracing: `strace ./program`
- Process monitoring: `ps`, `top`, `htop`
- File descriptor inspection: `ls -la /proc/PID/fd/`

## Contributing

When adding new examples:

1. Follow the existing folder structure
2. Include comprehensive README.md files
3. Provide clear compilation and execution instructions
4. Add relevant learning objectives and key concepts
