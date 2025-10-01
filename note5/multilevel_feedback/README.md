# Multi-Level Feedback Queue (MLFQ) Scheduling

## Introduction

The Multi-Level Feedback Queue (MLFQ) is a sophisticated CPU scheduling algorithm designed to address the limitations of simpler algorithms. Unlike SJF or STCF which require a priori knowledge of job length, MLFQ learns from past behavior to make future scheduling decisions.

## Objectives of MLFQ

1. **Optimize for Turnaround Time**: Complete short jobs quickly
2. **Optimize for Response Time**: Provide interactive responsiveness
3. **Learn from History**: Use past behavior to predict future needs
4. **Adapt to Changing Workloads**: Adjust priorities dynamically

## Basic Structure

MLFQ maintains multiple queues, each with a different priority level. Jobs move between queues based on their behavior.

### Basic Rules

1. **Priority-Based Selection**: If jobs exist at priority level A and priority level B (where A > B), the job at priority level A runs.
2. **Round-Robin Among Equal Priority**: If multiple jobs exist at the same priority level, run them in round-robin fashion.

## Dynamic Priority Adjustments

MLFQ adjusts job priorities based on their CPU usage patterns:

### Rule 3: New Job Assignment

- When a job enters the system, it is placed at the highest priority level.

### Rule 4a: Using Full Time Slice

- If a job uses its entire time quantum, its priority is decreased (moved down one queue).

### Rule 4b: Yielding the CPU

- If a job gives up the CPU before its time quantum expires (e.g., due to I/O), it remains at the same priority level.

## Illustrated Examples

### Example 1: Single Long-Running CPU-Bound Job

For a job that consistently uses its full time quantum:

```
Priority 2 |====|
Priority 1 |    ====|
Priority 0 |        ================>
           0    5    10   15   20   
```

- Job begins at highest priority
- After using full quantum, demoted to priority 1
- After another full quantum, demoted to priority 0
- Remains at lowest priority, running in round-robin fashion

### Example 2: Mix of Short Interactive Jobs and Long-Running Jobs

```
Priority 2 |A==|B=|C=|A=|B=|C=|A=|
Priority 1 |        |D=========|  
Priority 0 |                   |D==>
           0    5    10   15   20   
```

- A, B, C are short interactive jobs that yield the CPU before time quantum expires
- D is a long-running job that uses full time quanta
- Short jobs maintain high priority and get preference
- Long job eventually gets demoted to lowest priority

## Problems with Basic MLFQ

### 1. Starvation

- CPU-bound jobs can be stuck at lower priorities indefinitely
- Interactive jobs monopolize the CPU at high priorities

### 2. Gaming the Scheduler

- Programs can issue unnecessary I/O operations right before time quantum expiration to maintain high priority
- This artificially makes CPU-bound jobs appear I/O-bound

## Refinements to MLFQ

### Rule 5: Priority Boost

After a certain time period S, move all jobs to the highest priority queue.

**Benefits**:
- Prevents starvation of lower priority jobs
- Allows CPU-bound jobs that have waited long enough to get CPU time
- Corrects for misclassification of job types

### Rule 4 Enhancement: Better Accounting

Instead of resetting time quantum allocation when a job yields the CPU, track total time used at each level and demote when total exceeds quantum, regardless of how many times the job yields.

**Benefits**:
- Prevents gaming the scheduler via artificial I/O
- More accurate assessment of CPU usage patterns

## MLFQ Parameters

The behavior of MLFQ can be fine-tuned by adjusting:

1. **Number of Priority Queues**: Typically 3-5 for most systems
2. **Time Quantum Size**: Often increases exponentially with decreasing priority
   - Example: Priority 3 (10ms), Priority 2 (20ms), Priority 1 (40ms), Priority 0 (80ms)
3. **Boost Interval**: Frequency of priority resets (e.g., every 1 second)

## Approximating SJF

MLFQ approximates Shortest Job First without requiring knowledge of job lengths:
- Short jobs complete quickly at high priority levels
- Long jobs gradually move to lower priorities
- Interactive jobs stay at high priorities due to yielding behavior

![Multi-level Feedback Queue with Priority Adjustments (insert picture)]()

## Comparison to Other Algorithms

| Algorithm | Job Length Known | Response Time | Turnaround Time | Adaptive |
|-----------|------------------|---------------|-----------------|----------|
| FCFS      | No               | Poor          | Poor            | No       |
| SJF       | Yes              | Poor          | Excellent       | No       |
| STCF      | Yes              | Poor          | Excellent       | No       |
| RR        | No               | Good          | Poor            | No       |
| MLFQ      | No               | Good          | Good            | Yes      |

## Real-World Implementation

- Most modern operating systems use some variant of MLFQ
- Windows, Linux, and macOS all implement MLFQ-inspired schedulers
- Typically combined with priority boosting based on user interaction