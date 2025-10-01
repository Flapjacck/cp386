# CPU Scheduling

## Introduction

CPU scheduling is a fundamental component of operating systems that determines which processes get access to the CPU and for how long. The main goal is to efficiently allocate CPU time among competing processes to optimize system performance.

## The CPU Scheduling Problem

- **Problem Definition**: Given K jobs and N CPUs, how to assign jobs to CPUs?
- **Core Challenge**: Balance between efficiency, fairness, and responsiveness
- **Decision Points**: When does the OS make scheduling decisions?

### Scheduling Decision Points

1. **Process Creation**: When a new process is created
2. **Process Exit**: When a process terminates
3. **Process Blocking**: When a process requests I/O or waits for an event
4. **I/O Completion**: When an I/O operation completes

### Scheduling Types

| Type | Description | Decision Points |
|------|-------------|----------------|
| **Non-preemptive** | Once a process gets the CPU, it keeps it until termination or blocking | Process creation, exit, or voluntary yielding |
| **Preemptive** | OS can forcibly remove a process from CPU | All decision points plus timer interrupts |

## Scheduling Metrics

Operating systems evaluate scheduling algorithms based on various performance metrics:

- **Turnaround Time**: Time from job submission to completion
  - $T_{turnaround} = T_{completion} - T_{arrival}$

- **Response Time**: Time from job submission to first execution
  - $T_{response} = T_{first\_run} - T_{arrival}$

- **Waiting Time**: Time spent waiting in ready queue
  - $T_{waiting} = T_{turnaround} - T_{execution}$

- **Throughput**: Number of processes completed per unit time
  - $Throughput = \frac{n}{time}$ where n is number of completed processes

- **CPU Utilization**: Percentage of time CPU is busy
  - $Utilization = \frac{busy\_time}{total\_time} \times 100\%$

## Workload Assumptions

To simplify algorithm analysis, we often make the following assumptions:

1. Each job runs for the same amount of time
2. All jobs arrive at the same time
3. All jobs only use the CPU (no I/O)
4. Run time of each job is known in advance
5. No job dependencies

These assumptions are progressively relaxed for more realistic analysis.

## CPU Scheduling Algorithms

### First Come, First Served (FCFS/FIFO)

**Description**: Jobs are executed in order of arrival.

**Implementation**:
- Jobs placed in a simple FIFO queue
- Non-preemptive scheduling policy

**Example**:
```
Jobs: A (24ms), B (3ms), C (3ms)
```

Execution timeline:
```
A          B  C
|----------|--|--|
0         24 27 30
```

**Turnaround Time**:
- A: 24 - 0 = 24ms
- B: 27 - 0 = 27ms
- C: 30 - 0 = 30ms
- Average: (24 + 27 + 30) / 3 = 27ms

**Convoy Effect**: Short processes wait behind long processes, leading to poor average turnaround time.

### Shortest Job First (SJF)

**Description**: Execute the job with the smallest execution time first.

**Implementation**:
- Sort jobs by execution time
- Non-preemptive scheduling policy
- Optimal for minimizing average turnaround time (when all jobs arrive simultaneously)

**Example**:
```
Jobs: A (24ms), B (3ms), C (3ms)
```

Execution timeline:
```
B  C          A
|--|--|----------|
0  3  6         30
```

**Turnaround Time**:
- B: 3 - 0 = 3ms
- C: 6 - 0 = 6ms
- A: 30 - 0 = 30ms
- Average: (3 + 6 + 30) / 3 = 13ms

**Limitations**:
- Requires knowing job durations in advance
- Does not handle late arrivals optimally

### Shortest Time-to-Completion First (STCF) / Preemptive SJF (PSJF)

**Description**: Preemptive version of SJF. When new job arrives, compare remaining time of current job with execution time of new job.

**Implementation**:
- At each arrival, schedule job with shortest remaining time
- Preemptive scheduling policy

**Example**:
```
Jobs: A (arrives at 0, takes 24ms), B (arrives at 10, takes 3ms), C (arrives at 12, takes 3ms)
```

Execution timeline:
```
A          B  C  A
|----------|--|--|----|
0         10 13 16   30
```

**Turnaround Time**:
- A: 30 - 0 = 30ms
- B: 13 - 10 = 3ms
- C: 16 - 12 = 4ms
- Average: (30 + 3 + 4) / 3 = 12.33ms

### Round Robin (RR)

**Description**: Each process gets a small unit of CPU time (time quantum), then is preempted and placed at the end of the ready queue.

**Implementation**:
- FIFO queue with preemption after time quantum
- Jobs cycle through the queue

**Example**:
```
Jobs: A (24ms), B (3ms), C (3ms) with time quantum = 5ms
```

Execution timeline:
```
A    B    C    A    A    A    A    A
|----|----|----|----|----|----|----|---|
0    5    10   15   20   25   30   35  39
```

**Turnaround Time**:
- A: 39 - 0 = 39ms
- B: 10 - 0 = 10ms
- C: 15 - 0 = 15ms
- Average: (39 + 10 + 15) / 3 = 21.33ms

**Response Time**:
- A: 0 - 0 = 0ms
- B: 5 - 0 = 5ms
- C: 10 - 0 = 10ms
- Average: (0 + 5 + 10) / 3 = 5ms

**Quantum Size Tradeoffs**:
- Too small: excessive context switching overhead
- Too large: degenerates to FCFS
- Rule of thumb: quantum should be slightly larger than the time required for a typical interaction

![Time Quantum vs Turnaround Time (insert picture)]()

## Scheduling Trade-offs

### Turnaround Time vs Response Time

- **STCF**: Optimizes average turnaround time
- **Round Robin**: Optimizes average response time
- **Trade-off**: Cannot optimize both simultaneously

## Incorporating I/O

When processes perform I/O operations:

1. Process uses CPU for compute burst
2. Process issues I/O request and blocks
3. OS schedules another process on CPU
4. When I/O completes, process becomes ready again

**Goal**: Overlap CPU and I/O usage to maximize throughput

**I/O Scheduling Example**:
```
Process A: 10ms CPU, 10ms I/O, 10ms CPU
Process B: 10ms CPU
```

Without I/O overlap:
```
A-CPU   A-I/O   A-CPU   B-CPU
|-------|-------|-------|-------|
0      10      20      30      40
```

With I/O overlap:
```
A-CPU   A-CPU   
|-------|-------|
0      10      20
        B-CPU
        |-------|
        10      20
        A-I/O
        |-------|
        10      20
```

Total time: 20ms instead of 40ms (2x improvement)

## Summary

| Algorithm | Advantage | Disadvantage | Best For |
|-----------|-----------|--------------|----------|
| FCFS | Simple | Poor avg. turnaround (convoy effect) | Batch systems |
| SJF | Optimal turnaround time | Requires knowing job lengths | Batch systems with known workloads |
| STCF | Handles variable arrivals | Poor response time for long jobs | Batch processing |
| Round Robin | Good response time | Higher context switching overhead | Interactive systems |

## Real-World Considerations

- Most modern systems use multi-level feedback queues (combining benefits of different algorithms)
- I/O bound and CPU bound processes require different treatment
- Priority-based scheduling adds complexity but addresses business requirements