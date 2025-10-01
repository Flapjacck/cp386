# Synchronization and Locks

## Introduction to Concurrency

Concurrent programming allows multiple execution flows to progress simultaneously. While concurrency offers better resource utilization and performance, it introduces complex synchronization challenges.

## Race Conditions: The Core Problem

A race condition occurs when multiple threads of execution access shared data and at least one of them modifies it, with the final result depending on the relative timing of their execution.

### Classic Counter Example

Consider a simple shared counter increment:

```c
// Global shared variable
int counter = 0;

void increment_counter() {
    counter = counter + 1;
}

// Thread 1 and Thread 2 both call increment_counter()
```

This simple operation actually involves three steps:

1. Read the current value of counter
2. Add 1 to this value
3. Store the result back to counter

If two threads execute this function concurrently:

```text
Time | Thread 1                | Thread 2
-----|-------------------------|-------------------------
1    | Read counter (value=0)  |
2    |                         | Read counter (value=0)
3    | Add 1 → result=1        |
4    |                         | Add 1 → result=1
5    | Store 1 to counter      |
6    |                         | Store 1 to counter
```

Expected result: counter = 2
Actual result: counter = 1

This is a race condition - the final value depends on the exact timing of execution.

## Critical Sections

A critical section is a segment of code that accesses shared resources and must not be executed by more than one thread at a time.

### Requirements for Critical Section Solutions

1. **Mutual Exclusion**: Only one thread can execute in the critical section at any time
2. **Progress**: If no thread is in the critical section, a thread that wants to enter must be able to do so
3. **Bounded Waiting**: A thread waiting to enter the critical section will eventually do so (no starvation)

## Atomicity

**Atomicity** refers to operations that complete in a single step without the possibility of interruption.

- Most machine-level instructions are atomic (e.g., reading/writing a word)
- Complex operations like increment (read-modify-write) are not atomic
- Need for higher-level atomic constructs

## Locks: Basic Concept

A lock is a synchronization mechanism that ensures mutual exclusion for critical sections.

### Lock States and Operations

- **Available/Unlocked**: No thread currently holds the lock
- **Acquired/Locked**: A thread holds the lock and is executing the critical section

### Basic Lock API

```c
lock_t mutex;  // Lock variable

// Operations
lock(&mutex);    // Acquire lock (blocks if already locked)
unlock(&mutex);  // Release lock
```

Using locks to protect the counter example:

```c
int counter = 0;
lock_t mutex;

void safe_increment() {
    lock(&mutex);      // Acquire lock
    counter = counter + 1;
    unlock(&mutex);    // Release lock
}
```

## Building Locks

### Approach 1: Disabling Interrupts

```c
void lock() {
    disable_interrupts();
}

void unlock() {
    enable_interrupts();
}
```

**Advantages**:

- Simple implementation
- Guaranteed mutual exclusion

**Disadvantages**:

- Requires privilege (kernel mode)
- Doesn't work on multiprocessors
- Can cause system responsiveness issues
- Vulnerable to infinite loops or bugs in critical section

### Approach 2: Test-and-Set Spin Lock

Uses hardware atomic instruction Test-and-Set (also called atomic exchange):

```c
int TestAndSet(int *old_ptr, int new) {
    int old = *old_ptr;   // Read old value
    *old_ptr = new;       // Store new value
    return old;           // Return old value
}
```

Test-and-Set is implemented as a single, atomic CPU instruction.

Spin lock implementation:

```c
typedef struct {
    int flag;  // 0: unlocked, 1: locked
} lock_t;

void init_lock(lock_t *lock) {
    lock->flag = 0;  // Initially unlocked
}

void lock(lock_t *lock) {
    while (TestAndSet(&lock->flag, 1) == 1)
        ; // Spin - do nothing
}

void unlock(lock_t *lock) {
    lock->flag = 0;
}
```

**Advantages**:

- Works on multiprocessors
- Simple implementation

**Disadvantages**:

- Wastes CPU cycles while spinning
- No fairness guarantee (possible starvation)
- Cache coherence traffic

### Approach 3: Ticket Locks

Uses Fetch-and-Add atomic instruction to ensure FIFO ordering:

```c
int FetchAndAdd(int *ptr, int increment) {
    int old = *ptr;
    *ptr = old + increment;
    return old;
}
```

Ticket lock implementation:

```c
typedef struct {
    int ticket;     // Next ticket to be issued
    int serving;    // Currently serving ticket
} ticket_lock_t;

void init_lock(ticket_lock_t *lock) {
    lock->ticket = 0;
    lock->serving = 0;
}

void lock(ticket_lock_t *lock) {
    int my_ticket = FetchAndAdd(&lock->ticket, 1);
    while (lock->serving != my_ticket)
        ; // Spin until my turn
}

void unlock(ticket_lock_t *lock) {
    lock->serving++;
}
```

**Advantages**:

- Guarantees FIFO ordering (no starvation)
- Fair allocation of the lock

**Disadvantages**:

- Still wastes CPU while spinning
- More complex implementation

## Spin Locks vs. Sleep Locks

### Spin Locks

- Thread actively waits by spinning in a loop
- Wastes CPU cycles while waiting
- Good for short critical sections
- No context switch overhead

### Sleep Locks (OS-Assisted Locks)

- Thread is put to sleep when lock is unavailable
- Woken up when lock becomes available
- Efficient for longer waits
- Requires OS support (system calls)

**Mutex Implementation with OS Support**:

```c
// Simplified implementation of sleep-based mutex
typedef struct {
    int locked;         // Lock state
    queue_t wait_queue; // Queue of waiting threads
} mutex_t;

void mutex_lock(mutex_t *mutex) {
    disable_interrupts();
    if (mutex->locked) {
        // Add current thread to wait queue
        add_to_wait_queue(mutex->wait_queue, current_thread);
        // Put thread to sleep (releases CPU)
        sleep();
        // When woken up, we own the mutex
    } else {
        mutex->locked = 1;
    }
    enable_interrupts();
}

void mutex_unlock(mutex_t *mutex) {
    disable_interrupts();
    if (!is_empty(mutex->wait_queue)) {
        // Wake up first waiting thread
        thread_t *t = remove_from_wait_queue(mutex->wait_queue);
        wakeup(t);
    } else {
        mutex->locked = 0;
    }
    enable_interrupts();
}
```

## Lock Evaluation Criteria

1. **Mutual Exclusion**: Does it prevent multiple threads from entering the critical section?
2. **Fairness**: Does it prevent starvation?
3. **Performance**:
   - Overhead when uncontended
   - Overhead under contention
   - Cache effects
   - Scalability with thread count

## POSIX Thread (Pthread) Mutexes

POSIX threads provide a standard API for mutex operations:

```c
#include <pthread.h>

// Declaration and initialization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// or
pthread_mutex_t mutex;
pthread_mutex_init(&mutex, NULL);

// Lock operations
pthread_mutex_lock(&mutex);    // Block until mutex is available
pthread_mutex_trylock(&mutex); // Non-blocking attempt to lock
pthread_mutex_timedlock(&mutex, &timeout); // Lock with timeout

// Unlock operation
pthread_mutex_unlock(&mutex);

// Cleanup
pthread_mutex_destroy(&mutex);
```

## Advanced Synchronization Concepts

### 1. Reader-Writer Locks

Allow multiple readers to access data simultaneously, but only one writer can have exclusive access.

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Reader code
pthread_rwlock_rdlock(&rwlock);
// Read shared data
pthread_rwlock_unlock(&rwlock);

// Writer code
pthread_rwlock_wrlock(&rwlock);
// Modify shared data
pthread_rwlock_unlock(&rwlock);
```

### 2. Recursive Locks

Allow the same thread to lock the mutex multiple times without deadlock.

```c
pthread_mutexattr_t attr;
pthread_mutex_t recursive_mutex;

pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&recursive_mutex, &attr);
```

### 3. Condition Variables

Allow threads to wait for a specific condition to become true.

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

// Thread 1: Waits for condition
pthread_mutex_lock(&mutex);
while (!ready) {
    pthread_cond_wait(&cond, &mutex);  // Atomically releases mutex and waits
}
// Condition is now true
pthread_mutex_unlock(&mutex);

// Thread 2: Signals condition
pthread_mutex_lock(&mutex);
ready = 1;
pthread_cond_signal(&cond);  // Wake up a waiting thread
pthread_mutex_unlock(&mutex);
```

## Common Synchronization Problems

### 1. Deadlock

Occurs when two or more threads are waiting indefinitely for resources held by each other.

**Example**:

```c
// Thread 1
pthread_mutex_lock(&mutex_A);
pthread_mutex_lock(&mutex_B);  // May deadlock if Thread 2 holds mutex_B

// Thread 2
pthread_mutex_lock(&mutex_B);
pthread_mutex_lock(&mutex_A);  // May deadlock if Thread 1 holds mutex_A
```

**Prevention Strategies**:

- Always acquire locks in the same order
- Use trylock with timeout and retry
- Deadlock detection mechanisms

### 2. Priority Inversion

Lower priority thread holds a lock needed by a higher priority thread.

**Solution**: Priority inheritance - temporarily boost the priority of the lock holder.

### 3. Convoying

Threads pile up waiting for a lock, causing bursts of activity and idle periods.

**Solution**: Use multiple locks for different parts of data structure, reducing contention.

![Race Condition Execution Timeline (insert picture)]()

![Ticket Lock (insert picture)]()

## Summary

- **Race Conditions** occur when multiple threads access and modify shared data with unpredictable timing
- **Critical Sections** must be protected by synchronization primitives
- **Locks** provide mutual exclusion to prevent concurrent access to shared resources
- **Building Locks** requires hardware support for atomic operations
- **Lock Types** include spin locks, sleep locks, ticket locks, each with different trade-offs
- **Proper Lock Usage** requires careful planning to avoid deadlocks and other concurrency issues
