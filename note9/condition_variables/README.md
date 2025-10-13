# Condition Variables

## Introduction to Condition Variables

Condition variables are synchronization primitives that enable threads to wait until a specific condition is satisfied. They are used to solve ordering problems in concurrent programming where one thread must wait for a particular state change made by another thread.

## The Need for Condition Variables

While locks provide mutual exclusion for critical sections, they don't provide a mechanism for threads to wait for specific conditions. Consider these approaches without condition variables:

1. **Busy waiting (spinning)**:

```c
while (condition == false) {
    /* do nothing, just waste CPU cycles */
}
```

This approach wastes CPU resources by constantly checking the condition.

2. **Sleep and retry**:

```c
while (condition == false) {
    sleep(1);  /* sleep for some time */
}
```

This approach wastes less CPU but introduces arbitrary delays.

Condition variables solve these problems by allowing a thread to efficiently wait until another thread signals that a specific condition is true.

## Condition Variable Operations

Condition variables are always used with a mutex (lock) because:

1. The condition check involves accessing shared data
2. The lock-condition relationship ensures atomic operations

### Core Operations

1. **pthread_cond_wait(cond, mutex)**:
   - Atomically releases the mutex
   - Puts the calling thread to sleep on the condition variable
   - When the thread wakes up, it automatically reacquires the mutex before returning

2. **pthread_cond_signal()**:
   - Wakes up at most one thread waiting on the condition variable
   - If multiple threads are waiting, which one wakes is undefined

3. **pthread_cond_broadcast()**:
   - Wakes up all threads waiting on the condition variable
   - Useful when all waiting threads should check the condition

## Condition Variables in Practice

### The Standard Pattern

```c
pthread_mutex_lock(&mutex);
while (condition == false) {
    pthread_cond_wait(&cond, &mutex);
}
// Condition is now true, take appropriate action
pthread_mutex_unlock(&mutex);
```

### Why Use a While Loop?

The while loop is crucial for these reasons:

1. **Spurious wakeups**: A thread may occasionally wake up without a signal
2. **Multiple waiters**: Another thread might process the condition first
3. **False signals**: A thread may signal without making the condition true

## Common Use Cases

1. **Producer-Consumer Problems**: Coordinating producers and consumers sharing a buffer
2. **Reader-Writer Problems**: Managing priority between readers and writers
3. **Thread Joining**: Waiting for another thread to complete
4. **Task Scheduling**: Waiting for resources or specific system states

## Implementation Notes

- Condition variables don't maintain state or "remember" signals
- If a signal is sent when no threads are waiting, it's lost
- Always check conditions in a while loop, not an if statement
- Always use condition variables with their associated mutex

## Conclusion

Condition variables provide an efficient mechanism for threads to wait for specific conditions, avoiding the CPU waste of busy waiting and the arbitrary delays of sleep-and-retry approaches. When used properly with mutexes and while loops, they enable robust solutions to complex thread coordination problems.
