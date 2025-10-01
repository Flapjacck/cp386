# Thread Management

## Thread Creation and Termination

Threads are created and managed differently depending on the programming language and operating system. This section focuses primarily on POSIX threads (pthreads), which are widely used in Unix-like operating systems.

### POSIX Threads (pthreads)

#### Thread Creation

In C, threads are created using the `pthread_create()` function:

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                  void *(*start_routine) (void *), void *arg);
```

Parameters:

- `thread`: Pointer to store the thread ID
- `attr`: Thread attributes (or NULL for defaults)
- `start_routine`: Function to execute in the new thread
- `arg`: Argument to pass to the thread function

Example:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread_function(void *arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d is running\n", thread_id);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;
    
    // Create two threads
    pthread_create(&thread1, NULL, thread_function, &id1);
    pthread_create(&thread2, NULL, thread_function, &id2);
    
    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("Both threads have completed\n");
    return 0;
}
```

#### Thread Termination

Threads can terminate in several ways:

1. **Return from start function**:
   ```c
   void *thread_function(void *arg) {
       // Thread work
       return (void*)result;  // Return a result
   }
   ```

2. **Explicit call to pthread_exit()**:
   ```c
   void *thread_function(void *arg) {
       // Thread work
       pthread_exit((void*)result);  // Exit with result
   }
   ```

3. **Thread cancellation** (by another thread):
   ```c
   pthread_cancel(thread_id);
   ```

4. **Process termination** (all threads end when process ends)

#### Waiting for Thread Completion

The `pthread_join()` function waits for a thread to terminate:

```c
int pthread_join(pthread_t thread, void **retval);
```

Parameters:
- `thread`: Thread ID to wait for
- `retval`: Pointer to store the thread's return value

Example:

```c
void *result;
pthread_join(thread_id, &result);
printf("Thread returned: %ld\n", (long)result);
```

#### Detached Threads

By default, threads are joinable, meaning their resources aren't fully released until another thread calls `pthread_join()`. Detached threads automatically clean up when they terminate:

```c
// Method 1: Set attribute before creation
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thread, &attr, function, arg);
pthread_attr_destroy(&attr);

// Method 2: Detach after creation
pthread_detach(thread_id);
```

## Thread Attributes

Thread attributes control various aspects of thread behavior:

### 1. Stack Size

Control the size of a thread's stack:

```c
pthread_attr_t attr;
size_t stacksize = 2048 * 1024;  // 2MB

pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, stacksize);
pthread_create(&thread, &attr, function, arg);
pthread_attr_destroy(&attr);
```

### 2. Scheduling Policy and Priority

Control how threads are scheduled:

```c
pthread_attr_t attr;
struct sched_param param;

pthread_attr_init(&attr);
pthread_attr_setschedpolicy(&attr, SCHED_FIFO);  // Real-time scheduling
param.sched_priority = 20;
pthread_attr_setschedparam(&attr, &param);
pthread_create(&thread, &attr, function, arg);
pthread_attr_destroy(&attr);
```

Common scheduling policies include:

- `SCHED_OTHER`: Default time-sharing scheduler
- `SCHED_FIFO`: First-in, first-out real-time scheduler
- `SCHED_RR`: Round-robin real-time scheduler

### 3. Inheritance of Scheduling Attributes

Determine whether a thread inherits scheduling attributes from its parent:

```c
pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);  // Inherit from parent
// or
pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);  // Use explicit settings
```

## Thread-Specific Data (TSD)

Thread-specific data allows each thread to have its own copy of a variable:

```c
#include <pthread.h>

// Create a key for thread-specific data
pthread_key_t key;

// Initialize the key (usually done once)
pthread_key_create(&key, destructor_function);

// Set thread-specific data
pthread_setspecific(key, value);

// Get thread-specific data
void *value = pthread_getspecific(key);

// Delete the key when no longer needed
pthread_key_delete(key);
```

Example use case:

- Error return values like `errno`
- Thread-local caches
- Context information for thread-unsafe libraries

## Thread Pools

Thread pools manage a collection of worker threads to efficiently execute tasks:

### Benefits of Thread Pools

1. **Reduced creation overhead**: Threads are created once and reused
2. **Controlled concurrency**: Limit the number of active threads
3. **Improved performance**: Better throughput with less overhead
4. **Predictable resource usage**: Fixed number of threads

### Basic Thread Pool Implementation

```c
typedef struct {
    void (*function)(void *);  // Function to execute
    void *argument;            // Argument for the function
} task_t;

typedef struct {
    task_t *queue;             // Task queue
    int queue_size;            // Size of queue
    int head, tail, count;     // Queue management
    pthread_mutex_t lock;      // Queue lock
    pthread_cond_t not_empty;  // Signal when queue has tasks
    pthread_cond_t not_full;   // Signal when queue has space
    pthread_t *threads;        // Array of worker threads
    int num_threads;           // Number of threads
    int shutdown;              // Shutdown flag
} threadpool_t;

// Worker thread function
void *worker_thread(void *arg) {
    threadpool_t *pool = (threadpool_t *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->lock);
        
        // Wait for work
        while (pool->count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        }
        
        // Check for shutdown
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }
        
        // Get task from queue
        task_t task = pool->queue[pool->head];
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count--;
        
        // Signal that queue is not full
        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->lock);
        
        // Execute task
        task.function(task.argument);
    }
    
    return NULL;
}

// Create a thread pool
threadpool_t *threadpool_create(int num_threads, int queue_size) {
    // Initialize pool structure, create threads, etc.
    // ...
}

// Add task to thread pool
int threadpool_add_task(threadpool_t *pool, 
                        void (*function)(void *), void *argument) {
    pthread_mutex_lock(&pool->lock);
    
    // Wait if queue is full
    while (pool->count == pool->queue_size) {
        pthread_cond_wait(&pool->not_full, &pool->lock);
    }
    
    // Add task to queue
    pool->queue[pool->tail] = (task_t){function, argument};
    pool->tail = (pool->tail + 1) % pool->queue_size;
    pool->count++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
    
    return 0;
}
```

## Thread Synchronization

When multiple threads access shared data, synchronization is required to prevent race conditions.

### Mutex (Mutual Exclusion)

Mutexes ensure that only one thread can access a critical section at a time:

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_function(void *arg) {
    // Lock the mutex
    pthread_mutex_lock(&mutex);
    
    // Critical section - only one thread can be here at a time
    // ...
    
    // Unlock the mutex
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}
```

### Condition Variables

Condition variables allow threads to wait for a specific condition to become true:

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

// Producer thread
void *producer(void *arg) {
    // Prepare data
    
    pthread_mutex_lock(&mutex);
    ready = 1;  // Set condition
    pthread_cond_signal(&cond);  // Signal condition is true
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

// Consumer thread
void *consumer(void *arg) {
    pthread_mutex_lock(&mutex);
    
    while (!ready) {  // Test condition
        // Wait for condition (atomically releases mutex while waiting)
        pthread_cond_wait(&cond, &mutex);
    }
    
    // Process data
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}
```

### Read-Write Locks

Read-write locks allow multiple readers or a single writer:

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Reader thread
void *reader(void *arg) {
    pthread_rwlock_rdlock(&rwlock);  // Acquire read lock
    // Read shared data
    pthread_rwlock_unlock(&rwlock);  // Release lock
    return NULL;
}

// Writer thread
void *writer(void *arg) {
    pthread_rwlock_wrlock(&rwlock);  // Acquire write lock
    // Modify shared data
    pthread_rwlock_unlock(&rwlock);  // Release lock
    return NULL;
}
```

### Semaphores

Semaphores control access to a limited number of resources:

```c
#include <semaphore.h>

sem_t semaphore;

// Initialize semaphore with count 3
sem_init(&semaphore, 0, 3);

void *thread_function(void *arg) {
    // Wait for a resource
    sem_wait(&semaphore);
    
    // Use the resource
    
    // Release the resource
    sem_post(&semaphore);
    
    return NULL;
}
```

## Common Thread Patterns

### 1. Producer-Consumer Pattern

```c
#define BUFFER_SIZE 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

int buffer[BUFFER_SIZE];
int count = 0, in = 0, out = 0;

void *producer(void *arg) {
    int item;
    
    while (1) {
        item = produce_item();  // Create an item
        
        pthread_mutex_lock(&mutex);
        
        while (count == BUFFER_SIZE) {  // Buffer is full
            pthread_cond_wait(&not_full, &mutex);
        }
        
        buffer[in] = item;  // Add item to buffer
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        pthread_cond_signal(&not_empty);  // Signal consumers
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg) {
    int item;
    
    while (1) {
        pthread_mutex_lock(&mutex);
        
        while (count == 0) {  // Buffer is empty
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        item = buffer[out];  // Get item from buffer
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        pthread_cond_signal(&not_full);  // Signal producers
        pthread_mutex_unlock(&mutex);
        
        consume_item(item);  // Process the item
    }
}
```

### 2. Reader-Writer Pattern

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t turn = PTHREAD_COND_INITIALIZER;
int readers = 0;
bool writer_active = false;

void *reader(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        
        while (writer_active) {  // Wait if writer is active
            pthread_cond_wait(&turn, &mutex);
        }
        
        readers++;  // Increase reader count
        pthread_mutex_unlock(&mutex);
        
        // Read data
        
        pthread_mutex_lock(&mutex);
        readers--;  // Decrease reader count
        
        if (readers == 0) {  // Last reader signals waiting writers
            pthread_cond_broadcast(&turn);
        }
        
        pthread_mutex_unlock(&mutex);
    }
}

void *writer(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        
        while (readers > 0 || writer_active) {  // Wait if readers or another writer
            pthread_cond_wait(&turn, &mutex);
        }
        
        writer_active = true;  // Mark writer as active
        pthread_mutex_unlock(&mutex);
        
        // Write data
        
        pthread_mutex_lock(&mutex);
        writer_active = false;  // Mark writer as inactive
        pthread_cond_broadcast(&turn);  // Signal waiting threads
        pthread_mutex_unlock(&mutex);
    }
}
```

### 3. Thread Pool Pattern

See the thread pool implementation above.

## Thread Debugging Techniques

Debugging multithreaded applications can be challenging due to their non-deterministic nature.

### 1. Thread-Aware Debugging Tools

- **GDB**: Use `info threads`, `thread <id>` commands
- **Valgrind**: Detects memory errors in multithreaded applications
- **Helgrind**: Identifies synchronization issues

### 2. Thread Logging

Create a thread-safe logging mechanism:

```c
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_safe_log(const char *format, ...) {
    pthread_mutex_lock(&log_mutex);
    
    // Get thread ID
    pthread_t thread_id = pthread_self();
    
    // Print timestamp and thread ID
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    printf("[%02d:%02d:%02d][Thread %lu] ",
           local->tm_hour, local->tm_min, local->tm_sec,
           (unsigned long)thread_id);
    
    // Print the message
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    pthread_mutex_unlock(&log_mutex);
}
```

### 3. Static Analysis Tools

- Thread-safety checkers
- Race detectors
- Deadlock analyzers

## Best Practices for Thread Programming

### 1. Minimize Shared Data

- Keep shared data to a minimum
- Use thread-local storage where possible
- Pass data via message passing rather than shared memory

### 2. Synchronization Guidelines

- Keep critical sections short
- Avoid nested locks to prevent deadlocks
- Always acquire locks in the same order
- Use higher-level synchronization constructs when possible

### 3. Thread Creation and Management

- Create an appropriate number of threads (usually near the number of CPU cores)
- Use thread pools for tasks rather than creating/destroying threads
- Consider thread affinity for performance-critical applications

### 4. Error Handling

- Check return values from thread functions
- Implement proper cleanup in case of errors
- Use cancellation points and handlers carefully

### 5. Thread Safety in Libraries

When designing thread-safe libraries:

- Make functions reentrant
- Avoid global variables
- Use thread-local storage where necessary
- Document synchronization requirements