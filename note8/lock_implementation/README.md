# Lock Implementation and Design

## Introduction to Locks

Locks are synchronization primitives that provide mutual exclusion to protect critical sections of code. They ensure that only one thread can access a shared resource at a time, preventing race conditions and maintaining data consistency.

## Lock Requirements

An effective lock implementation must provide:

1. **Mutual Exclusion**: Only one thread can hold the lock at a time
2. **Progress**: If no thread holds the lock, a thread attempting to acquire it should succeed
3. **Bounded Waiting**: Threads waiting for a lock should eventually acquire it
4. **Performance**: Minimal overhead, especially in uncontended cases

## Building Locks: From Simple to Sophisticated

### Approach 1: Disable Interrupts

The simplest approach to mutual exclusion is to disable interrupts during critical sections:

```c
void lock() {
    disable_interrupts();
}

void unlock() {
    enable_interrupts();
}
```

#### Advantages:

- Simple to implement
- Guaranteed mutual exclusion on single-processor systems

#### Disadvantages:

- Requires privileged execution (kernel mode)
- Does not work on multiprocessor systems
- Can cause system responsiveness issues
- Vulnerable to bugs in critical section (entire system freezes)

### Approach 2: Software-Based Solutions

#### Peterson's Algorithm

A software solution for two threads:

```c
// Shared variables
int turn = 0;           // Whose turn is it?
bool flag[2] = {false, false};  // Is thread ready to enter critical section?

void lock(int thread_id) {
    int other = 1 - thread_id;  // ID of the other thread
    
    flag[thread_id] = true;     // I'm interested in entering
    turn = other;               // Give priority to the other thread
    
    // Wait until the other thread is not interested or it's my turn
    while (flag[other] && turn == other) {
        // Busy-wait (spin)
    }
}

void unlock(int thread_id) {
    flag[thread_id] = false;    // I'm no longer interested
}
```

#### Advantages:

- Works without hardware support
- Provides mutual exclusion and progress

#### Disadvantages:

- Limited to two threads
- Relies on busy-waiting (wastes CPU)
- Assumes sequential memory consistency

### Approach 3: Hardware Atomic Instructions

Modern processors provide atomic instructions that can be used to implement locks:

#### Test-and-Set (TAS)

```c
// Atomic hardware instruction
int TestAndSet(int *target, int value) {
    int old = *target;  // Read old value
    *target = value;    // Store new value
    return old;         // Return old value
}

// Using TAS for a spinlock
typedef struct {
    int flag;  // 0: unlocked, 1: locked
} lock_t;

void init_lock(lock_t *lock) {
    lock->flag = 0;  // Initially unlocked
}

void lock(lock_t *lock) {
    while (TestAndSet(&lock->flag, 1) == 1) {
        // Spin - do nothing
    }
}

void unlock(lock_t *lock) {
    lock->flag = 0;
}
```

#### Compare-and-Swap (CAS)

```c
// Atomic hardware instruction
int CompareAndSwap(int *ptr, int expected, int new_value) {
    int actual = *ptr;
    if (actual == expected) {
        *ptr = new_value;
    }
    return actual;
}

// Using CAS for a spinlock
void lock(lock_t *lock) {
    while (CompareAndSwap(&lock->flag, 0, 1) != 0) {
        // Spin - do nothing
    }
}
```

#### Advantages:

- Works on multiprocessor systems
- Relatively simple implementation
- Support for atomic operations in hardware

#### Disadvantages:

- Still uses busy-waiting (spin)
- No fairness guarantee (possible starvation)
- Cache coherence traffic can become problematic

### Approach 4: Ticket Locks

Ticket locks ensure first-come, first-served order:

```c
typedef struct {
    int next_ticket;   // Next ticket to be issued
    int now_serving;   // Currently serving ticket
} ticket_lock_t;

void init_lock(ticket_lock_t *lock) {
    lock->next_ticket = 0;
    lock->now_serving = 0;
}

// Atomic fetch-and-add
int fetch_and_add(int *ptr, int increment) {
    int old = *ptr;
    *ptr = old + increment;
    return old;
}

void lock(ticket_lock_t *lock) {
    int my_ticket = fetch_and_add(&lock->next_ticket, 1);
    while (lock->now_serving != my_ticket) {
        // Spin until my turn
    }
}

void unlock(ticket_lock_t *lock) {
    lock->now_serving++;
}
```

#### Advantages:

- FIFO ordering (fairness)
- Prevents starvation
- Relatively simple implementation

#### Disadvantages:

- Still uses busy-waiting
- Can cause cache line ping-ponging

## Spin Lock Optimizations

### 1. Test and Test-and-Set (TTAS)

Reduces cache coherence traffic by first reading the lock value without writing:

```c
void lock(lock_t *lock) {
    while (1) {
        // First test without writing (cache-friendly)
        while (lock->flag == 1) {
            // Keep spinning on cached read
        }
        
        // Now try to acquire the lock with atomic TAS
        if (TestAndSet(&lock->flag, 1) == 0) {
            // Lock acquired
            break;
        }
    }
}
```

### 2. Exponential Backoff

Reduces contention by adding increasing delays between attempts:

```c
void lock_with_backoff(lock_t *lock) {
    int backoff = 1;
    
    while (1) {
        if (TestAndSet(&lock->flag, 1) == 0) {
            // Lock acquired
            break;
        }
        
        // Backoff for random time between 0 and backoff
        int delay = rand() % backoff;
        for (int i = 0; i < delay; i++) {
            // CPU delay
        }
        
        // Increase backoff time (capped)
        backoff = min(backoff * 2, MAX_BACKOFF);
    }
}
```

### 3. MCS Locks

Named after Mellor-Crummey and Scott, these locks create an explicit queue of waiters:

```c
typedef struct qnode {
    struct qnode *next;
    bool locked;
} qnode;

typedef struct {
    qnode *tail;  // Last node in the queue
} mcs_lock_t;

void lock(mcs_lock_t *lock, qnode *my_node) {
    my_node->next = NULL;
    my_node->locked = true;
    
    // Atomically exchange tail and return old value
    qnode *predecessor = atomic_exchange(&lock->tail, my_node);
    
    if (predecessor != NULL) {
        // I'm not first in line, wait until predecessor signals
        predecessor->next = my_node;
        while (my_node->locked) {
            // Spin on local variable (cache-friendly)
        }
    }
}

void unlock(mcs_lock_t *lock, qnode *my_node) {
    if (my_node->next == NULL) {
        // No one waiting behind me
        if (atomic_compare_exchange(&lock->tail, my_node, NULL)) {
            // Lock released
            return;
        }
        
        // Wait for successor to link in
        while (my_node->next == NULL) {
            // Spin
        }
    }
    
    // Signal successor
    my_node->next->locked = false;
}
```

#### Advantages:

- Each thread spins on a local variable (better cache behavior)
- FIFO fairness
- Better scalability with many cores

## Beyond Spin Locks: OS-Assisted Locks

When critical sections are long or contention is high, yielding the CPU is more efficient than spinning.

### Mutex with Futex (Linux)

```c
// User-space mutex implementation using Linux futex
typedef struct {
    int locked;  // 0: unlocked, 1: locked
} mutex_t;

void mutex_init(mutex_t *mutex) {
    mutex->locked = 0;
}

void mutex_lock(mutex_t *mutex) {
    int expected = 0;
    
    // First try to acquire the lock without system call
    if (atomic_compare_exchange(&mutex->locked, &expected, 1)) {
        // Lock acquired in user space
        return;
    }
    
    // Lock is contended, call into the kernel
    while (1) {
        // If lock is already taken (1), wait
        if (mutex->locked == 1) {
            futex_wait(&mutex->locked, 1);
            continue;
        }
        
        // Try to acquire the lock
        expected = 0;
        if (atomic_compare_exchange(&mutex->locked, &expected, 1)) {
            // Lock acquired
            return;
        }
    }
}

void mutex_unlock(mutex_t *mutex) {
    // Release the lock
    mutex->locked = 0;
    
    // Wake up a waiting thread, if any
    futex_wake(&mutex->locked, 1);
}
```

### Advantages of OS-Assisted Locks:

- Threads yield the CPU instead of busy-waiting
- Better system-wide efficiency
- Appropriate for longer critical sections

### Disadvantages:

- System call overhead
- Context switching overhead

## Reader-Writer Locks

Reader-writer locks allow multiple readers or a single writer:

```c
typedef struct {
    int readers;       // Number of active readers
    bool writer_active; // Is a writer active?
    mutex_t mutex;     // Mutex for updating reader count
    sem_t no_readers;  // Semaphore to wait for readers to finish
} rwlock_t;

void rwlock_init(rwlock_t *lock) {
    lock->readers = 0;
    lock->writer_active = false;
    mutex_init(&lock->mutex);
    sem_init(&lock->no_readers, 0, 1); // Binary semaphore
}

void read_lock(rwlock_t *lock) {
    // Wait if a writer is active
    while (lock->writer_active) {
        // Busy-wait or yield
    }
    
    // Increment reader count
    mutex_lock(&lock->mutex);
    lock->readers++;
    if (lock->readers == 1) {
        // First reader acquires the semaphore
        sem_wait(&lock->no_readers);
    }
    mutex_unlock(&lock->mutex);
}

void read_unlock(rwlock_t *lock) {
    mutex_lock(&lock->mutex);
    lock->readers--;
    if (lock->readers == 0) {
        // Last reader releases the semaphore
        sem_post(&lock->no_readers);
    }
    mutex_unlock(&lock->mutex);
}

void write_lock(rwlock_t *lock) {
    lock->writer_active = true;  // Prevent new readers
    sem_wait(&lock->no_readers); // Wait for readers to finish
}

void write_unlock(rwlock_t *lock) {
    lock->writer_active = false; // Allow readers again
    sem_post(&lock->no_readers); // Release the semaphore
}
```

### Trade-offs:

- More complex than simple locks
- Can improve performance for read-heavy workloads
- Potential for writer starvation

## Advanced Lock Concepts

### 1. Recursive Locks

Allows the same thread to acquire the lock multiple times:

```c
typedef struct {
    int owner;       // Thread ID of owner (0 if unlocked)
    int count;       // Number of times locked
    mutex_t mutex;   // Underlying mutex
} recursive_mutex_t;

void recursive_mutex_lock(recursive_mutex_t *mutex) {
    int thread_id = get_thread_id();
    
    if (mutex->owner == thread_id) {
        // Already own the lock, just increment count
        mutex->count++;
        return;
    }
    
    // Don't own the lock, acquire it
    mutex_lock(&mutex->mutex);
    mutex->owner = thread_id;
    mutex->count = 1;
}

void recursive_mutex_unlock(recursive_mutex_t *mutex) {
    int thread_id = get_thread_id();
    
    if (mutex->owner != thread_id) {
        // Error: not owner
        return;
    }
    
    mutex->count--;
    if (mutex->count == 0) {
        // Last unlock releases the mutex
        mutex->owner = 0;
        mutex_unlock(&mutex->mutex);
    }
}
```

### 2. Condition Variables

Allow threads to wait for specific conditions:

```c
// Example of consumer waiting for data
mutex_lock(&mutex);
while (buffer_empty()) {
    // Atomically release mutex and wait
    cond_wait(&data_available, &mutex);
    // Upon return, mutex is reacquired
}
// Process data
mutex_unlock(&mutex);

// Producer adding data
mutex_lock(&mutex);
add_data_to_buffer();
// Signal waiting consumer
cond_signal(&data_available);
mutex_unlock(&mutex);
```

### 3. Read-Copy-Update (RCU)

A synchronization technique favoring readers:

- Readers don't acquire locks, just read
- Writers create a new copy, update it, then atomically switch pointers
- Wait for all readers to finish before freeing old data

## Lock-Free Techniques

### 1. Compare-and-Swap Loop

```c
bool update_value(int *address, int new_value) {
    int old_value;
    do {
        old_value = *address;  // Read current value
        // If someone else modified it, retry
    } while (!atomic_compare_exchange(address, &old_value, new_value));
    return true;
}
```

### 2. Optimistic Concurrency Control

Read, compute, then verify nothing changed before committing:

```c
bool optimistic_update() {
    // Read the current state
    State current = read_current_state();
    
    // Compute changes without locking
    State new_state = compute_new_state(current);
    
    // Try to commit changes atomically
    return atomic_compare_exchange(&state, &current, new_state);
    // If fails, can retry or abort
}
```

## Lock Performance Considerations

### 1. Lock Granularity

- **Coarse-grained**: Few locks covering large data structures
  - Simpler code but more contention
- **Fine-grained**: Many locks for small parts of data
  - Less contention but more complexity, potential deadlocks

### 2. Lock Contention Metrics

- **Acquisition time**: How long it takes to acquire the lock
- **Hold time**: How long locks are held
- **Contention rate**: How often lock acquisition fails/blocks

### 3. Performance Tuning

- Use profiling tools to identify hot locks
- Consider alternative algorithms with less synchronization
- Partition data to reduce contention

## Lock Implementation Best Practices

1. **Use existing libraries** when possible
2. **Match lock type** to access pattern
3. **Keep critical sections short**
4. **Document lock ordering** to prevent deadlocks
5. **Use lock hierarchies** for complex systems
6. **Consider lock-free alternatives** for performance-critical paths

## Real-World Lock Implementations

### 1. POSIX Pthread Mutex

```c
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_operation() {
    pthread_mutex_lock(&mutex);
    // Critical section
    pthread_mutex_unlock(&mutex);
}
```

### 2. Java Synchronized Blocks

```java
class SafeCounter {
    private int count = 0;
    
    public synchronized void increment() {
        count++;
    }
    
    public synchronized int getCount() {
        return count;
    }
}
```

### 3. Linux Kernel Spinlocks

```c
spinlock_t lock;

spin_lock_init(&lock);

// In critical path
spin_lock(&lock);
// Critical section
spin_unlock(&lock);
```

## Summary

- **Lock design involves trade-offs** between simplicity, performance, and fairness
- **Hardware support** enables efficient lock implementations
- **Spin locks** are good for short critical sections
- **OS-assisted locks** are better for longer waits
- **Advanced lock types** address specific synchronization patterns
- **Lock-free techniques** can provide higher scalability in some cases
- **Careful design** is required to avoid deadlocks and contention
