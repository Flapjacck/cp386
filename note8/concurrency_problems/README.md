# Race Conditions and Concurrency Problems

## Introduction to Race Conditions

A race condition occurs when multiple threads or processes access and manipulate the same data concurrently, and the final outcome depends on the precise timing of their execution. Race conditions are one of the most common and difficult-to-debug problems in concurrent programming.

## What Makes Race Conditions Problematic

Race conditions are particularly challenging because:

1. **Non-deterministic behavior**: The same code may work correctly sometimes and fail other times
2. **Hard to reproduce**: Issues may only appear under specific timing conditions
3. **Hidden in plain sight**: The code may look correct at first glance

## Anatomy of a Race Condition

### The Classic Counter Example

Consider a shared counter being incremented by multiple threads:

```c
// Global shared variable
int counter = 0;

void increment_counter() {
    counter = counter + 1;  // Not atomic!
}
```

This simple operation actually involves three steps:

1. Read the current value of counter from memory
2. Add 1 to this value
3. Store the result back to counter

If two threads execute this function concurrently:

| Time | Thread A | Thread B |
|------|----------|----------|
| 1    | Read counter (0) | |
| 2    | | Read counter (0) |
| 3    | Add 1 (result: 1) | |
| 4    | | Add 1 (result: 1) |
| 5    | Write 1 to counter | |
| 6    | | Write 1 to counter |

The final value of counter is 1, not 2 as expected.

## Types of Concurrency Problems

### 1. Read-Write Race Conditions

When one thread reads a value while another is in the process of modifying it, leading to inconsistent views of the data.

### 2. Write-Write Race Conditions

When multiple threads attempt to modify the same data simultaneously, potentially overwriting each other's changes.

### 3. Check-Then-Act Race Conditions

When a thread checks a condition and then acts based on it, but the condition changes between the check and the action.

```c
// Example of check-then-act race condition
if (resource_available) {
    use_resource();  // Another thread might take the resource between check and use
}
```

### 4. Time-of-Check-to-Time-of-Use (TOCTOU)

A specific type of race condition where a resource is checked for availability and then used, but its state changes between the check and use.

## Critical Sections

A critical section is a segment of code that accesses a shared resource and must not be executed by more than one thread simultaneously.

### Properties of Critical Section Solutions

1. **Mutual Exclusion**: Only one thread can execute in the critical section at a time
2. **Progress**: If no thread is in the critical section, a waiting thread must be able to enter
3. **Bounded Waiting**: A thread waiting to enter the critical section will eventually do so

## Real-World Race Condition Examples

### Banking Systems

```
Initial balance: $1000
Thread A: Withdraw $500
Thread B: Withdraw $800
```

Without proper synchronization:

1. Thread A reads balance: $1000
2. Thread B reads balance: $1000
3. Thread A calculates new balance: $500
4. Thread B calculates new balance: $200
5. Thread A updates balance: $500
6. Thread B updates balance: $200

Result: Account has $200, but $1300 was withdrawn!

### Web Applications

Race conditions in web applications can lead to serious security vulnerabilities:

1. **Double submission**: A user submits a form twice quickly
2. **Authentication bypass**: Timing attacks on authentication systems
3. **CSRF vulnerabilities**: Multiple concurrent requests manipulating state

## The Lost Update Problem

When two transactions read and then update the same data, one transaction's update might be overwritten by the other.

### Database Example

```sql
-- Transaction A
SELECT balance FROM accounts WHERE id = 123;  -- Returns 1000
-- Calculate new balance: 1000 - 100 = 900
UPDATE accounts SET balance = 900 WHERE id = 123;

-- Transaction B (executed concurrently)
SELECT balance FROM accounts WHERE id = 123;  -- Also returns 1000
-- Calculate new balance: 1000 - 200 = 800
UPDATE accounts SET balance = 800 WHERE id = 123;
```

The final balance would be $800, not $700 as expected.

## The Phantom Read Problem

When a transaction reads a set of rows that satisfy a condition, but a concurrent transaction modifies the data, causing the first transaction to see additional rows in a subsequent query.

## Solutions to Race Conditions

### 1. Atomic Operations

Use hardware-supported atomic operations for simple cases:

```c
// Atomic increment (hardware supported)
atomic_increment(&counter);
```

### 2. Mutual Exclusion

Use locks or mutexes to protect critical sections:

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_increment() {
    pthread_mutex_lock(&mutex);
    counter = counter + 1;
    pthread_mutex_unlock(&mutex);
}
```

### 3. Optimistic Concurrency Control

Read data, calculate changes, then verify nothing has changed before committing:

```c
bool update_with_retry() {
    int max_retries = 5;
    int retry_count = 0;
    
    while (retry_count < max_retries) {
        int original_value = get_current_value();
        int new_value = calculate_new_value(original_value);
        
        if (compare_and_swap(&value, original_value, new_value)) {
            return true;  // Success
        }
        
        retry_count++;
    }
    
    return false;  // Failed after max retries
}
```

### 4. Transaction Isolation Levels

In database systems, different isolation levels control how transactions interact:

- **READ UNCOMMITTED**: No isolation, can read uncommitted changes
- **READ COMMITTED**: Can only read committed data
- **REPEATABLE READ**: Re-reading same data gives same results
- **SERIALIZABLE**: Transactions behave as if executed serially

## Deadlocks: When Solutions Cause New Problems

While solving race conditions, improper use of locks can create deadlocks.

### The Dining Philosophers Problem

A classic example of deadlock potential:

- Five philosophers sit at a round table
- Each needs two forks to eat (one on each side)
- If each picks up the left fork first, all can be waiting for right forks

### Deadlock Conditions

Four conditions must exist for deadlock:

1. **Mutual Exclusion**: Resources cannot be shared
2. **Hold and Wait**: Processes hold resources while waiting for others
3. **No Preemption**: Resources cannot be forcibly taken away
4. **Circular Wait**: A circular chain of processes, each waiting for resources held by the next

### Deadlock Prevention Strategies

1. **Lock Ordering**: Always acquire locks in a consistent order
2. **Lock Timeouts**: Set maximum time to wait for a lock
3. **Deadlock Detection**: Periodically check for deadlock conditions
4. **Resource Hierarchy**: Assign global ordering to all resources

## Starvation

When threads are perpetually denied access to resources, preventing progress.

### Causes of Starvation

1. **Unfair scheduling policies**
2. **Priority inversion**
3. **Poorly designed resource allocation**

### Prevention

1. **Using fair locks**: Locks that guarantee FIFO ordering
2. **Aging**: Gradually increasing priority of waiting processes
3. **Priority inheritance**: Temporarily boosting priority of lock holders

## Livelock

When threads keep changing their state in response to each other, preventing progress.

### Example

```text
Person A and Person B meet in a hallway:
- A steps to his left to let B pass
- B steps to her left at the same time
- Now they're still blocking each other
- A steps to his right to try again
- B steps to her right at the same time
- This continues indefinitely
```

### Prevention

1. **Randomized back-off**: Add random delays before retry
2. **Resource arbitration**: Use third-party decision maker

## Testing and Debugging Concurrency Issues

### Techniques

1. **Static analysis tools**: Find potential race conditions in code
2. **Thread sanitizers**: Detect data races during execution
3. **Stress testing**: Run with many threads under high load
4. **Forced thread interleaving**: Deliberately try problematic thread orderings

### Design Principles for Concurrency

1. **Immutability**: Use immutable data structures where possible
2. **Thread confinement**: Keep data within single threads
3. **Synchronization**: Use appropriate synchronization mechanisms
4. **Minimizing shared state**: Reduce the amount of shared data

## Summary

- **Race conditions** occur when multiple threads access shared data without synchronization
- **Critical sections** must be protected to prevent concurrent access
- **Deadlocks, livelocks, and starvation** are higher-level concurrency problems
- **Various solutions** exist, each with their own trade-offs
- **Prevention is better than cure**: Design systems to avoid concurrency issues from the start

## Further Reading

- "The Little Book of Semaphores" by Allen Downey
- "Java Concurrency in Practice" by Brian Goetz
- "Operating Systems: Three Easy Pieces" by Remzi Arpaci-Dusseau and Andrea Arpaci-Dusseau
