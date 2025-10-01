# Multi-Level Feedback Queue (MLFQ) Scheduling

## Introduction

The Multi-Level Feedback Queue (MLFQ) is a sophisticated CPU scheduling algorithm designed to address the limitations of simpler algorithms. Unlike SJF or STCF which require a priori knowledge of job length, MLFQ learns from past behavior to make future scheduling decisions.

## Objectives of MLFQ

1. **Optimize for Turnaround Time**: Complete short jobs quickly
2. **Optimize for Response Time**: Provide interactive responsiveness
3. **Adapt to Changing Workloads**: Learn from past process behavior

## How MLFQ Works

MLFQ uses multiple queues, each with a different priority level. Processes move between these queues based on their behavior:

1. **New processes start at the highest priority queue**
2. **Processes that use their entire time slice move to a lower priority queue**
3. **Processes that relinquish the CPU before their time slice ends remain at the same priority**
4. **Periodically, all processes are moved to the highest priority queue (to prevent starvation)**

### The Basic Rules

1. **Rule 1**: If Priority(A) > Priority(B), then process A runs (B doesn't)
2. **Rule 2**: If Priority(A) = Priority(B), then A & B run in RR fashion
3. **Rule 3**: When a job enters the system, it is placed at the highest priority
4. **Rule 4**: Once a job uses up its time allotment at a given level, its priority is reduced
5. **Rule 5**: After some time period S, move all jobs to the highest priority queue

## Priority Boosting

Without Rule 5, long-running processes could be stuck at lower priorities indefinitely. Periodic boosting:

- Prevents starvation of long-running processes
- Protects against priority inversion (when low-priority processes block high-priority ones)
- Allows the system to adapt if a process changes behavior

## Gaming the Scheduler

Without the proper safeguards, users could "game" the MLFQ scheduler:

- A process could issue an I/O operation just before its time slice ends
- This would allow it to stay at a high priority level
- To prevent this, the scheduler tracks total runtime across queue levels

## Varying Time Slices Across Queues

A common MLFQ implementation uses different time quantum sizes at each level:

- Highest priority queues: Shorter time slices (e.g., 10ms)
- Lower priority queues: Longer time slices (e.g., 20ms, 40ms, etc.)

This approach provides:

- Quick response for interactive processes
- Lower overhead for CPU-bound processes

## MLFQ Parameters

The effectiveness of an MLFQ scheduler depends on several parameters:

- **Number of Queues**: More queues allow finer-grained priorities but add complexity
- **Time Quantum for Each Queue**: Balancing responsiveness vs. overhead
- **Boosting Time Period (S)**: How often to reset priorities
- **Method for Adjusting Priorities**: How quickly processes move down queues

## Example: MLFQ in Practice

Consider a system with three priority queues (high, medium, low):

1. **Queue 0 (High Priority)**: Time quantum = 8ms
2. **Queue 1 (Medium Priority)**: Time quantum = 16ms
3. **Queue 2 (Low Priority)**: Time quantum = 32ms

Process behavior:

- **Interactive processes**: Make frequent I/O requests, remain in high priority
- **CPU-bound processes**: Use full time slices, gradually move to lowest priority
- **Every 100ms**: All processes boosted to highest priority

## MLFQ Advantages

1. **No Prior Knowledge Required**: Unlike SJF, doesn't need to know job lengths
2. **Adaptability**: Adjusts to changing process behavior
3. **Balances Metrics**: Good for both turnaround time and response time
4. **Fairness**: Priority boosting ensures all processes eventually get CPU time

## MLFQ Disadvantages

1. **Complexity**: More complex to implement than simpler algorithms
2. **Parameter Tuning**: Effectiveness depends on properly set parameters
3. **Overhead**: Queue management and priority adjustments add overhead
4. **Gaming Potential**: Can still be manipulated without proper safeguards

## Real-World MLFQ Implementation

### Solaris Time-Sharing Scheduler (TS)

The Solaris operating system implements a variant of MLFQ:

- 60 priority levels (0-59, with 59 being highest)
- Processes start at priority level 40
- Multiple queues with different time slices
- Priority is adjusted based on CPU usage history
- A priority boost mechanism prevents starvation

### FreeBSD ULE Scheduler

FreeBSD's ULE scheduler:

- Four priority queues for time-sharing processes
- Interactive processes get higher priorities
- CPU usage measured in "ticks" determines priority adjustments
- Periodic priority recalculation

## MLFQ in Modern Systems

Modern systems have built on MLFQ concepts:

- **Linux Completely Fair Scheduler (CFS)**: Not a pure MLFQ but incorporates concepts like dynamic priority adjustment
- **Windows**: Uses a multi-level feedback queue with 32 priority levels
- **macOS**: Incorporates MLFQ principles with the addition of thread categorization

## Best Practices for Process Design with MLFQ

Developers should be aware of how their applications interact with schedulers:

- **Batch processes**: Design to run efficiently at lower priorities
- **Interactive processes**: Relinquish CPU when waiting for input
- **Background services**: Structure to avoid competing with foreground tasks

## Summary

MLFQ represents an elegant solution to the scheduling problem:

- **Learns from the past** to predict future process behavior
- **Balances turnaround and response time** requirements
- **Adapts to different process types** without prior knowledge
- **Prevents starvation** through periodic priority boosting
- **Forms the basis** of many real-world scheduling implementations

The algorithm's principles continue to influence modern scheduler design, demonstrating how systems can adapt to workloads without requiring explicit information about process behavior.
