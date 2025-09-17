# CPU Virtualization: The Illusion of Multiple CPUs

## 🎯 Overview

CPU virtualization is one of the three fundamental pillars of modern operating systems (alongside memory virtualization and persistent storage). This demonstration shows how an operating system creates the **illusion** that each process has its own dedicated CPU, when in reality multiple processes are sharing the same physical CPU through sophisticated time-sharing mechanisms.

## 🧠 Core Concepts

### What is CPU Virtualization?

CPU virtualization is the OS mechanism that allows multiple processes to share a single CPU while giving each process the illusion of having exclusive access to the processor. This is achieved through:

1. **Time-sharing**: Rapidly switching CPU execution between processes
2. **Preemptive scheduling**: OS forcibly interrupts processes to ensure fairness
3. **Context switching**: Saving and restoring process state during switches
4. **Virtual CPU abstraction**: Each process sees a continuous execution model

### The Scheduler's Role

The **CPU scheduler** is the OS component responsible for:

```
┌─────────────────────────────────────────────────────────────┐
│                    CPU SCHEDULER                            │
├─────────────────────────────────────────────────────────────┤
│  1. Decide which process runs next                          │
│  2. Determine how long each process runs (time quantum)     │
│  3. Handle interruptions and context switches              │
│  4. Maintain fairness and system responsiveness           │
│  5. Optimize for throughput and latency                   │
└─────────────────────────────────────────────────────────────┘
```

## 📊 Scheduling Algorithms in Detail

Based on the [OSDev Wiki Scheduling Algorithms](https://wiki.osdev.org/Scheduling_Algorithms), modern systems implement several types of schedulers:

### 1. Round-Robin Scheduling

The simplest preemptive scheduling algorithm:

```
Time Quantum = 20-50ms (typical)

Process Queue: [A] → [B] → [C] → [A] → [B] → [C] → ...
                ↑
            Currently Running

Timeline:
0ms    A runs
50ms   A preempted → B runs  
100ms  B preempted → C runs
150ms  C preempted → A runs
200ms  A preempted → B runs
...
```

**Advantages:**
- Simple implementation
- Fair allocation of CPU time
- Good for interactive systems

**Disadvantages:**
- No priority system
- Context switching overhead
- May not be optimal for all workloads

### 2. Priority-Based Round-Robin

Modern Linux systems use variations of this:

```
High Priority Queue:    [System Tasks]
Medium Priority Queue:  [User Interactive]  
Low Priority Queue:     [Background Jobs]

Scheduling Rule: Always run highest priority available process
Within same priority: Use round-robin
```

### 3. Completely Fair Scheduler (CFS)

Linux's current default scheduler:

```
Virtual Runtime (vruntime) based scheduling:
- Each process has a vruntime counter
- Process with lowest vruntime runs next
- Running process accumulates vruntime
- Ensures long-term fairness

Red-Black Tree Structure:
        Process C (vruntime: 150ms)
       /                         \
Process A                    Process D  
(vruntime: 100ms)           (vruntime: 200ms)
                            /
                     Process B
                    (vruntime: 175ms)
```

## 🔍 Visual Process Timeline

When running multiple instances of our CPU demo:

```
Time →  0s    1s    2s    3s    4s    5s    6s    7s    8s
        │     │     │     │     │     │     │     │     │
Process A: ████░░░░████░░░░████░░░░████░░░░████░░░░████░░░░
Process B: ░░░░████░░░░████░░░░████░░░░████░░░░████░░░░████
Process C: ░░░░░░░░░░░░░░░░████░░░░░░░░████░░░░░░░░████░░░░

Legend:
████ = Process running (consuming CPU)
░░░░ = Process waiting (not scheduled)

Observation: Each process gets roughly equal CPU time over time,
but execution is interleaved rather than continuous.
```

## 💻 Hardware Support for Scheduling

Modern CPUs provide hardware support for efficient scheduling:

### Timer Interrupts

```c
// Simplified timer interrupt handler (in kernel)
void timer_interrupt_handler() {
    current_process->cpu_time_used++;
    
    if (current_process->cpu_time_used >= TIME_QUANTUM) {
        // Time quantum expired - preempt current process
        save_process_context(current_process);
        current_process = scheduler_pick_next();
        restore_process_context(current_process);
        current_process->cpu_time_used = 0;
    }
    
    acknowledge_interrupt();
}
```

### Context Switch Mechanism

```
Context Switch Steps:
1. Save current process state:
   - CPU registers (EAX, EBX, EIP, ESP, etc.)
   - Program counter
   - Stack pointer
   - Memory management info

2. Update process state:
   - RUNNING → READY
   - Update scheduling data structures

3. Select next process:
   - Run scheduling algorithm
   - Pick highest priority ready process

4. Restore next process state:
   - Load saved registers
   - Restore memory mappings
   - Update hardware state

5. Resume execution:
   - Jump to saved program counter
   - Process continues unaware of interruption
```

## 🚀 How to Compile and Run

### Compilation

```bash
# Individual compilation
gcc -Wall -Wextra -std=c99 -o cpu cpu.c

# Using the project Makefile
make note1
# or
make all
```

### Single Process Demo

```bash
./cpu A
```

**Expected Output:**
```
[PID 12345] Starting CPU virtualization demo with identifier 'A'
[PID 12345] This process will run in an infinite loop, printing 'A'
[PID 12345] OS scheduler will interleave this with other processes
[PID 12345] Use Ctrl+C or 'killall cpu' to stop

[PID 12345][Iter      1][Time     1.00s] A
[PID 12345][Iter      2][Time     2.00s] A
[PID 12345][Iter      3][Time     3.00s] A
...
```

### Multiple Process Demo (The Real Magic!)

```bash
# Start multiple background processes
./cpu A & ./cpu B & ./cpu C & ./cpu D &
```

**Expected Interleaved Output:**
```
[PID 12345] Starting CPU virtualization demo with identifier 'A'
[PID 12346] Starting CPU virtualization demo with identifier 'B'
[PID 12347] Starting CPU virtualization demo with identifier 'C'
[PID 12348] Starting CPU virtualization demo with identifier 'D'

[PID 12345][Iter      1][Time     1.03s] A
[PID 12346][Iter      1][Time     1.05s] B
[PID 12347][Iter      1][Time     1.08s] C
[PID 12348][Iter      1][Time     1.11s] D
[PID 12345][Iter      2][Time     2.06s] A
[PID 12346][Iter      2][Time     2.09s] B
[PID 12347][Iter      2][Time     2.12s] C
[PID 12348][Iter      2][Time     2.15s] D
...
```

**Key Observations:**
- Output is **interleaved**, not sequential
- Each process thinks it's running continuously
- Timing shows scheduling overhead (not exactly 1.00s intervals)
- All processes make progress over time

### Performance Analysis

```bash
# Monitor system performance while running
top -p $(pgrep cpu | tr '\n' ',' | sed 's/,$//')

# Watch context switches
vmstat 1

# Detailed process monitoring
watch -n 1 'ps -eo pid,ppid,cmd,pcpu,time'
```

## 🧪 Experimental Variations

### Experiment 1: CPU Affinity
```bash
# Pin processes to specific CPU cores
taskset -c 0 ./cpu A &
taskset -c 1 ./cpu B &
taskset -c 0 ./cpu C &  # Shares core with A
taskset -c 1 ./cpu D &  # Shares core with B
```

### Experiment 2: Process Priorities
```bash
# Run with different priorities
nice -n -10 ./cpu HIGH_PRIORITY &  # Higher priority
nice -n 10 ./cpu LOW_PRIORITY &    # Lower priority
./cpu NORMAL_PRIORITY &            # Default priority
```

### Experiment 3: Real-time Scheduling
```bash
# Requires root privileges
sudo chrt --fifo 99 ./cpu REALTIME &
./cpu NORMAL &
```

## 🔬 Understanding the Results

### Why Do We See Interleaving?

1. **Timer Interrupts**: Hardware timer interrupts occur every few milliseconds
2. **Preemptive Scheduling**: OS forces context switches even if process doesn't yield
3. **Fairness**: Scheduler ensures all processes get CPU time
4. **No Process Monopolization**: No single process can hog the CPU indefinitely

### Measuring Context Switch Overhead

```bash
# Use perf to measure context switches
perf stat -e context-switches ./cpu A &
perf stat -e context-switches ./cpu B &
perf stat -e context-switches ./cpu C &

# Let run for 30 seconds then kill
sleep 30
killall cpu
```

### Load Average Impact

```bash
# Check system load before
uptime

# Run CPU-intensive processes
./cpu A & ./cpu B & ./cpu C & ./cpu D &

# Check load during execution
uptime  # Should show load approaching number of CPU cores
```

## 🎓 Educational Value

This demonstration illustrates several fundamental OS concepts:

### 1. **Abstraction**
- Processes see a simple, continuous execution model
- OS handles complex scheduling underneath

### 2. **Virtualization**  
- Multiple virtual CPUs from one physical CPU
- Each process has illusion of dedicated hardware

### 3. **Resource Management**
- Fair allocation of CPU time
- Prevention of resource monopolization

### 4. **Concurrency**
- Multiple processes executing "simultaneously"
- Coordination without explicit synchronization

## 🔗 References and Further Reading

### Academic Resources
- **OSTEP Chapter 7**: [CPU Scheduling](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf)
- **OSTEP Chapter 6**: [Limited Direct Execution](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-mechanisms.pdf)

### Technical Documentation
- [OSDev Wiki: Scheduling Algorithms](https://wiki.osdev.org/Scheduling_Algorithms)
- [Linux CFS Scheduler Documentation](https://www.kernel.org/doc/Documentation/scheduler/sched-design-CFS.txt)
- [Intel 64 and IA-32 Architectures Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)

### Real-World Examples
- **Linux**: `/proc/schedstat` - scheduler statistics
- **Windows**: Task Manager → Performance → CPU
- **macOS**: Activity Monitor → CPU usage

## 🛠️ Cleanup

```bash
# Stop all CPU demo processes
killall cpu

# Or stop specific background jobs
jobs       # List background jobs
kill %1    # Kill job 1
kill %2    # Kill job 2
# etc.

# Force kill if necessary
pkill -f "cpu [A-Z]"
```

---

**🎉 Congratulations!** You've just witnessed CPU virtualization in action. The interleaved output proves that your operating system is successfully sharing CPU time between multiple processes, creating the fundamental illusion that makes modern computing possible.
D
...
```

The exact order will vary due to the non-deterministic nature of process scheduling.

## Learning Points

1. Each process thinks it has the CPU to itself
2. The OS rapidly switches between processes (context switching)
3. From the user's perspective, processes appear to run simultaneously
4. The `Spin(1)` function simulates work being done by the process
