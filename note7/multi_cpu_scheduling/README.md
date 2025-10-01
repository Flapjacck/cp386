# Multi-CPU Scheduling

## Introduction to Multicore Processors

Modern computing systems increasingly rely on multicore processors, where a single physical chip contains multiple processing units (cores). This architectural shift has significant implications for operating system design, particularly in the area of CPU scheduling.

## The Rise of Multicore

- **Moore's Law Limitations**: Physical constraints on clock speeds
- **Power and Heat Concerns**: Higher frequencies lead to increased heat generation
- **Parallel Processing Solution**: Multiple cores at lower frequencies provide better performance-per-watt
- **Core Proliferation**: From dual-core to many-core architectures (8, 16, 32+ cores)

## Cache Memory and Its Importance

### Cache Hierarchy

Modern processors use a hierarchical cache system:

| Level | Typical Size | Access Time | Shared/Private |
|-------|-------------|-------------|----------------|
| L1 Cache | 32-64 KB | 1-3 cycles | Private per core |
| L2 Cache | 256 KB - 1 MB | 10-20 cycles | Private or shared |
| L3 Cache | 2-50 MB | 40-60 cycles | Shared across cores |
| Main Memory | 4-512 GB | 100-300 cycles | Shared across system |

### Cache Locality Principles

1. **Temporal Locality**: Recently accessed data likely to be accessed again soon
2. **Spatial Locality**: Data near recently accessed locations likely to be accessed soon

### Cache Coherence Problem

In multicore systems, the same memory location might be cached in multiple CPU cores, creating potential inconsistency issues.

**Example Scenario**:
- Core 1 reads value A (value=0) into its cache
- Core 2 reads value A (value=0) into its cache
- Core 1 updates A to 1 in its cache
- Core 2 reads A from its cache (still sees 0!)

**Solution: Cache Coherence Protocols**
- **Write-through**: All writes immediately update main memory
- **Write-back with invalidation**: When one core writes to a cache line, other cores' copies are invalidated
- **MESI Protocol** (Modified, Exclusive, Shared, Invalid): Common coherence protocol that tracks cache line states

## Cache Affinity in Scheduling

**Cache Affinity**: The benefit of keeping a process on the same CPU where it was previously running.

**Importance**:
- Warm caches improve performance significantly
- Moving a process to another CPU causes "cold cache" misses
- Context switch costs are higher across different CPUs

## Multiprocessor Scheduling Approaches

### Single-Queue Multiprocessor Scheduling (SQMS)

**Characteristics**:
- One shared queue for all CPUs
- Centralized scheduling decisions
- Global synchronization required

**Advantages**:
- Simple implementation
- Natural load balancing
- Works well for homogeneous workloads

**Disadvantages**:
- Lock contention on shared queue
- Poor scalability with increasing core count
- Ignores cache affinity

**Implementation Example**:
```c
// Pseudo-code for SQMS
mutex_t queue_lock;

void schedule() {
    process_t *next_proc;
    
    mutex_lock(&queue_lock);
    next_proc = pick_next_process_from_global_queue();
    mutex_unlock(&queue_lock);
    
    run_process(next_proc);
}
```

### Multi-Queue Multiprocessor Scheduling (MQMS)

**Characteristics**:
- Separate queue per CPU
- Decentralized scheduling decisions
- Local synchronization only

**Advantages**:
- Better scalability
- Reduced lock contention
- Preserves cache affinity

**Disadvantages**:
- Potential load imbalance
- More complex implementation
- Requires additional mechanisms for load balancing

**Implementation Example**:
```c
// Pseudo-code for MQMS
// Each CPU has its own queue and lock
mutex_t queue_lock[MAX_CPUS];
queue_t proc_queue[MAX_CPUS];

void schedule(int cpu_id) {
    process_t *next_proc;
    
    mutex_lock(&queue_lock[cpu_id]);
    next_proc = pick_next_process_from_local_queue(cpu_id);
    mutex_unlock(&queue_lock[cpu_id]);
    
    run_process(next_proc);
}
```

## Load Balancing Techniques

### Key Challenges

- **Imbalance Detection**: Determining when CPUs have uneven workloads
- **Migration Cost**: Moving processes incurs cache invalidation overhead
- **Affinity vs. Balance**: Trade-off between cache affinity and load balance

### Load Balancing Strategies

1. **Push Migration**:
   - Periodically check for overloaded CPUs
   - Push excess processes to underloaded CPUs
   - Implemented via periodic balancing daemon

2. **Pull Migration (Work Stealing)**:
   - Idle CPUs pull work from busy CPUs' queues
   - Triggered when a CPU's run queue becomes empty
   - Preserves cache affinity when system is lightly loaded

3. **Initial Placement**:
   - Careful assignment of new processes to CPUs
   - Consider memory locality, CPU load, cache usage

**Example Work Stealing Algorithm**:
```c
// Pseudo-code for work stealing
void idle_cpu_find_work(int cpu_id) {
    int target_cpu = find_busiest_cpu();
    
    if (target_cpu != -1) {
        mutex_lock(&queue_lock[target_cpu]);
        process_t *stolen_proc = steal_process_from_queue(target_cpu);
        mutex_unlock(&queue_lock[target_cpu]);
        
        if (stolen_proc) {
            mutex_lock(&queue_lock[cpu_id]);
            add_to_local_queue(cpu_id, stolen_proc);
            mutex_unlock(&queue_lock[cpu_id]);
        }
    }
}
```

![Scheduling Queues with Work Stealing (insert picture)]()

## Advanced Multi-CPU Scheduling Topics

### Processor Affinity Types

- **Soft Affinity**: OS tries to keep process on same CPU but may migrate it
- **Hard Affinity**: Process explicitly bound to specific CPU(s)
- **NUMA Affinity**: Considering memory access costs in NUMA systems

### CPU Sets and Processor Binding

Modern operating systems provide APIs for explicit control of CPU affinity:

```c
// Linux example of CPU affinity control
#include <sched.h>

int main() {
    cpu_set_t cpu_set;
    
    // Clear the CPU set
    CPU_ZERO(&cpu_set);
    
    // Add CPU 0 to the set
    CPU_SET(0, &cpu_set);
    
    // Bind current process to CPU 0
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    
    /* Process now runs only on CPU 0 */
    
    return 0;
}
```

### Scheduling Domains

Linux organizes CPUs into hierarchical scheduling domains that reflect hardware topology:

- Socket level
- Core level
- Hyper-thread level

This hierarchy influences load balancing decisions and migration costs.

## Real-world Implementations

### Linux Completely Fair Scheduler (CFS)

- Per-CPU runqueues
- Red-black tree ordered by virtual runtime
- Load tracking and periodic balancing
- Task groups for hierarchical fairness

### Windows Scheduler

- Priority-based preemptive scheduling
- Processor groups for many-core systems
- Dynamic priority boosting
- Ideal processor tracking for affinity