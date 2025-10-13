# Semaphores

## Introduction to Semaphores

Semaphores are synchronization primitives that control access to resources by multiple processes or threads. They were invented by Edsger W. Dijkstra in 1965 and provide a more versatile mechanism than simple locks.

A semaphore is essentially a counter that can be incremented and decremented atomically, with the key property that when the counter is zero, attempts to decrement it will block until another thread increments it.

## Semaphore Properties

1. A semaphore is initialized with a non-negative integer value
2. Two operations can be performed on semaphores:
   - **wait** (P, down): Decrements the semaphore value
   - **post** (V, up): Increments the semaphore value
3. When a semaphore's value is zero, any thread calling wait() will block until another thread calls post()
4. The semaphore operations are atomic (they cannot be interrupted)

## Types of Semaphores

### Binary Semaphores

- Initialized with value 1
- Can only have values 0 and 1
- Functionally equivalent to a mutex
- Used for mutual exclusion (protecting critical sections)

### Counting Semaphores

- Initialized with value N (where N > 1)
- Used to manage access to a pool of N resources
- Represents the number of available resources
- Also used for synchronization between threads

### POSIX Semaphore Functions

```c
#include <semaphore.h>

// Initialize an unnamed semaphore
int sem_init(sem_t *sem, int pshared, unsigned int value);

// Wait operation (P/down) - decrements or blocks
int sem_wait(sem_t *sem);

// Non-blocking wait - returns error instead of blocking
int sem_trywait(sem_t *sem);

// Post operation (V/up) - increments and wakes waiters
int sem_post(sem_t *sem);

// Get current value of semaphore
int sem_getvalue(sem_t *sem, int *valp);

// Destroy a semaphore
int sem_destroy(sem_t *sem);
```

## Common Use Cases for Semaphores

### 1. Mutual Exclusion (Binary Semaphores)

```c
sem_t mutex;
sem_init(&mutex, 0, 1);  // Initialize to 1

// Thread function
void* worker(void* arg) {
    // Enter critical section
    sem_wait(&mutex);
    
    // Access shared resource
    // ...
    
    // Exit critical section
    sem_post(&mutex);
    
    return NULL;
}
```

### 2. Resource Counting

```c
#define MAX_CONNECTIONS 10
sem_t connection_pool;
sem_init(&connection_pool, 0, MAX_CONNECTIONS);

// Thread function
void* client_handler(void* arg) {
    // Acquire a connection from the pool
    sem_wait(&connection_pool);
    
    // Use the connection
    // ...
    
    // Return the connection to the pool
    sem_post(&connection_pool);
    
    return NULL;
}
```

### 3. Synchronization/Ordering

```c
sem_t sync;
sem_init(&sync, 0, 0);  // Initialize to 0

// Thread A
void* thread_a(void* arg) {
    // Do some work first
    // ...
    
    // Signal thread B that work is complete
    sem_post(&sync);
    return NULL;
}

// Thread B
void* thread_b(void* arg) {
    // Wait for thread A to complete its work
    sem_wait(&sync);
    
    // Now do thread B's work
    // ...
    
    return NULL;
}
```

## Semaphores vs Mutexes vs Condition Variables

| Feature | Semaphore | Mutex | Condition Variable |
|---------|-----------|-------|-------------------|
| Purpose | Resource counting & synchronization | Exclusive access | Wait for condition |
| Values | Can be > 1 | Binary (locked/unlocked) | No value (just a queue) |
| Owner | No concept of ownership | Has an owner | N/A |
| Release | Any thread can post | Only owner can unlock | N/A |
| Use with | Stand-alone | Critical sections | Always with mutex |

## Implementation Considerations

1. **Starvation**: With semaphores, there's no inherent FIFO guarantee for waiting threads
2. **Deadlock**: Incorrect use of semaphores can lead to deadlocks
3. **Lost Wakeups**: If sem_post() is called before sem_wait(), the signal may be lost
4. **Performance**: Semaphores involve kernel calls, which may have higher overhead than user-space synchronization

## Conclusion

Semaphores provide a powerful synchronization mechanism that can handle mutual exclusion, resource counting, and thread coordination. While they are more versatile than mutexes, this power comes with more responsibility to use them correctly. In many cases, higher-level abstractions like thread pools, concurrent data structures, or condition variables with mutexes may provide clearer solutions with less room for error.
