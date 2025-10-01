# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>

/*
 * multicore_scheduling.c - Demonstrates different multiprocessor scheduling approaches
 *
 * This program simulates both Single Queue Multiprocessor Scheduling (SQMS)
 * and Multi-Queue Multiprocessor Scheduling (MQMS) with load balancing.
 */

#define MAX_PROCESSES 20
#define MAX_CPUS 4
#define SIMULATION_TIME 100

typedef struct {
    int id;
    int burst_time;
    int remaining_time;
    int assigned_cpu;
    int completion_time;
    int last_cpu;       // Track last CPU used (for cache affinity)
    int cache_misses;   // Counter for cache misses (simplified simulation)
} Process;

// Global data structures
Process processes[MAX_PROCESSES];
int num_processes = 0;

// SQMS implementation
pthread_mutex_t global_queue_lock;
Process* global_queue[MAX_PROCESSES];
int global_queue_size = 0;

// MQMS implementation
pthread_mutex_t cpu_queue_lock[MAX_CPUS];
Process* cpu_queue[MAX_CPUS][MAX_PROCESSES];
int cpu_queue_size[MAX_CPUS] = {0};

// Initialize a process with given burst time
void init_process(Process* p, int id, int burst) {
    p->id = id;
    p->burst_time = burst;
    p->remaining_time = burst;
    p->assigned_cpu = -1;
    p->completion_time = -1;
    p->last_cpu = -1;
    p->cache_misses = 0;
}

// Add process to global queue (SQMS)
void sqms_add_process(Process* p) {
    pthread_mutex_lock(&global_queue_lock);
    global_queue[global_queue_size++] = p;
    pthread_mutex_unlock(&global_queue_lock);
}

// Get next process from global queue (SQMS)
Process* sqms_get_next_process() {
    Process* next = NULL;
    
    pthread_mutex_lock(&global_queue_lock);
    if (global_queue_size > 0) {
        next = global_queue[0];
        
        // Shift remaining processes
        for (int i = 0; i < global_queue_size - 1; i++) {
            global_queue[i] = global_queue[i + 1];
        }
        global_queue_size--;
    }
    pthread_mutex_unlock(&global_queue_lock);
    
    return next;
}

// Add process to CPU's local queue (MQMS)
void mqms_add_process(Process* p, int cpu_id) {
    pthread_mutex_lock(&cpu_queue_lock[cpu_id]);
    cpu_queue[cpu_id][cpu_queue_size[cpu_id]++] = p;
    pthread_mutex_unlock(&cpu_queue_lock[cpu_id]);
}

// Get next process from CPU's local queue (MQMS)
Process* mqms_get_next_process(int cpu_id) {
    Process* next = NULL;
    
    pthread_mutex_lock(&cpu_queue_lock[cpu_id]);
    if (cpu_queue_size[cpu_id] > 0) {
        next = cpu_queue[cpu_id][0];
        
        // Shift remaining processes
        for (int i = 0; i < cpu_queue_size[cpu_id] - 1; i++) {
            cpu_queue[cpu_id][i] = cpu_queue[cpu_id][i + 1];
        }
        cpu_queue_size[cpu_id]--;
    }
    pthread_mutex_unlock(&cpu_queue_lock[cpu_id]);
    
    return next;
}

// Work stealing algorithm for load balancing in MQMS
Process* mqms_steal_work(int cpu_id) {
    Process* stolen = NULL;
    int target_cpu = -1;
    int max_queue_size = 0;
    
    // Find the busiest CPU
    for (int i = 0; i < MAX_CPUS; i++) {
        if (i != cpu_id && cpu_queue_size[i] > max_queue_size) {
            max_queue_size = cpu_queue_size[i];
            target_cpu = i;
        }
    }
    
    // If we found a target with processes to steal
    if (target_cpu != -1 && max_queue_size > 1) {  // Leave at least one process
        pthread_mutex_lock(&cpu_queue_lock[target_cpu]);
        if (cpu_queue_size[target_cpu] > 1) {  // Double-check after acquiring lock
            // Steal from the end of the queue (least recently added)
            stolen = cpu_queue[target_cpu][cpu_queue_size[target_cpu] - 1];
            cpu_queue_size[target_cpu]--;
            printf("CPU %d steals Process %d from CPU %d\n", 
                   cpu_id, stolen->id, target_cpu);
        }
        pthread_mutex_unlock(&cpu_queue_lock[target_cpu]);
    }
    
    return stolen;
}

// Simulates cache effects of running a process on a CPU
void simulate_cache_effects(Process* p, int cpu_id) {
    if (p->last_cpu != -1 && p->last_cpu != cpu_id) {
        // Process migrated to a different CPU - simulate cache misses
        p->cache_misses += 10;  // Arbitrary penalty
        printf("Process %d migrated from CPU %d to CPU %d: cache miss penalty\n",
               p->id, p->last_cpu, cpu_id);
    }
    
    // Update last CPU
    p->last_cpu = cpu_id;
}

// SQMS Simulation function
void simulate_sqms() {
    printf("\n--- Single Queue Multiprocessor Scheduling Simulation ---\n\n");
    
    // Initialize global queue
    pthread_mutex_init(&global_queue_lock, NULL);
    global_queue_size = 0;
    
    // Reset processes
    for (int i = 0; i < num_processes; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = -1;
        processes[i].last_cpu = -1;
        processes[i].cache_misses = 0;
        sqms_add_process(&processes[i]);
    }
    
    // Main simulation loop
    int time = 0;
    int completed = 0;
    Process* current_proc[MAX_CPUS] = {NULL};
    int time_slice[MAX_CPUS] = {0};
    
    while (time < SIMULATION_TIME && completed < num_processes) {
        printf("Time %d:\n", time);
        
        // For each CPU
        for (int cpu = 0; cpu < MAX_CPUS; cpu++) {
            // If CPU is idle or current process time slice expired
            if (current_proc[cpu] == NULL || time_slice[cpu] <= 0) {
                // If there was a process, put it back if not completed
                if (current_proc[cpu] != NULL) {
                    if (current_proc[cpu]->remaining_time > 0) {
                        sqms_add_process(current_proc[cpu]);
                    }
                    current_proc[cpu] = NULL;
                }
                
                // Get next process
                current_proc[cpu] = sqms_get_next_process();
                
                if (current_proc[cpu] != NULL) {
                    simulate_cache_effects(current_proc[cpu], cpu);
                    printf("  CPU %d: Running Process %d (remaining: %d)\n", 
                           cpu, current_proc[cpu]->id, current_proc[cpu]->remaining_time);
                    time_slice[cpu] = 5;  // Time quantum of 5
                } else {
                    printf("  CPU %d: Idle\n", cpu);
                }
            } else {
                // Continue running current process
                printf("  CPU %d: Running Process %d (remaining: %d)\n", 
                       cpu, current_proc[cpu]->id, current_proc[cpu]->remaining_time);
                
                // Reduce remaining time
                current_proc[cpu]->remaining_time--;
                time_slice[cpu]--;
                
                // If process completed
                if (current_proc[cpu]->remaining_time <= 0) {
                    current_proc[cpu]->completion_time = time + 1;
                    printf("  CPU %d: Process %d completed\n", cpu, current_proc[cpu]->id);
                    completed++;
                    current_proc[cpu] = NULL;
                }
            }
        }
        
        time++;
        if (time % 10 == 0) printf("\n");  // Visual separator every 10 time units
    }
    
    printf("SQMS simulation completed. %d processes finished.\n", completed);
    
    // Print cache misses statistics
    int total_misses = 0;
    for (int i = 0; i < num_processes; i++) {
        total_misses += processes[i].cache_misses;
    }
    printf("Total cache misses in SQMS: %d\n\n", total_misses);
    
    pthread_mutex_destroy(&global_queue_lock);
}

// MQMS Simulation function
void simulate_mqms() {
    printf("\n--- Multi-Queue Multiprocessor Scheduling Simulation ---\n\n");
    
    // Initialize CPU queues
    for (int i = 0; i < MAX_CPUS; i++) {
        pthread_mutex_init(&cpu_queue_lock[i], NULL);
        cpu_queue_size[i] = 0;
    }
    
    // Reset processes and distribute among CPUs initially
    for (int i = 0; i < num_processes; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].completion_time = -1;
        processes[i].last_cpu = -1;
        processes[i].cache_misses = 0;
        
        // Simple initial distribution: round-robin among CPUs
        int target_cpu = i % MAX_CPUS;
        mqms_add_process(&processes[i], target_cpu);
    }
    
    // Main simulation loop
    int time = 0;
    int completed = 0;
    Process* current_proc[MAX_CPUS] = {NULL};
    int time_slice[MAX_CPUS] = {0};
    
    while (time < SIMULATION_TIME && completed < num_processes) {
        printf("Time %d:\n", time);
        
        // Periodically check and balance load (every 20 time units)
        if (time > 0 && time % 20 == 0) {
            printf("  --- Load balancing checkpoint ---\n");
            for (int cpu = 0; cpu < MAX_CPUS; cpu++) {
                printf("  CPU %d queue size: %d\n", cpu, cpu_queue_size[cpu]);
            }
        }
        
        // For each CPU
        for (int cpu = 0; cpu < MAX_CPUS; cpu++) {
            // If CPU is idle or current process time slice expired
            if (current_proc[cpu] == NULL || time_slice[cpu] <= 0) {
                // If there was a process, put it back if not completed
                if (current_proc[cpu] != NULL) {
                    if (current_proc[cpu]->remaining_time > 0) {
                        mqms_add_process(current_proc[cpu], cpu);  // Add back to local queue
                    }
                    current_proc[cpu] = NULL;
                }
                
                // Try to get process from local queue
                current_proc[cpu] = mqms_get_next_process(cpu);
                
                // If no local process, try work stealing
                if (current_proc[cpu] == NULL) {
                    current_proc[cpu] = mqms_steal_work(cpu);
                }
                
                if (current_proc[cpu] != NULL) {
                    simulate_cache_effects(current_proc[cpu], cpu);
                    printf("  CPU %d: Running Process %d (remaining: %d)\n", 
                           cpu, current_proc[cpu]->id, current_proc[cpu]->remaining_time);
                    time_slice[cpu] = 5;  // Time quantum of 5
                } else {
                    printf("  CPU %d: Idle\n", cpu);
                }
            } else {
                // Continue running current process
                printf("  CPU %d: Running Process %d (remaining: %d)\n", 
                       cpu, current_proc[cpu]->id, current_proc[cpu]->remaining_time);
                
                // Reduce remaining time
                current_proc[cpu]->remaining_time--;
                time_slice[cpu]--;
                
                // If process completed
                if (current_proc[cpu]->remaining_time <= 0) {
                    current_proc[cpu]->completion_time = time + 1;
                    printf("  CPU %d: Process %d completed\n", cpu, current_proc[cpu]->id);
                    completed++;
                    current_proc[cpu] = NULL;
                }
            }
        }
        
        time++;
        if (time % 10 == 0) printf("\n");  // Visual separator every 10 time units
    }
    
    printf("MQMS simulation completed. %d processes finished.\n", completed);
    
    // Print cache misses statistics
    int total_misses = 0;
    for (int i = 0; i < num_processes; i++) {
        total_misses += processes[i].cache_misses;
    }
    printf("Total cache misses in MQMS: %d\n\n", total_misses);
    
    // Clean up
    for (int i = 0; i < MAX_CPUS; i++) {
        pthread_mutex_destroy(&cpu_queue_lock[i]);
    }
}

int main() {
    printf("Multi-CPU Scheduling Simulation\n");
    printf("================================\n");
    printf("This program compares SQMS and MQMS scheduling approaches with load balancing.\n\n");
    
    // Create sample processes with different burst times
    num_processes = 12;
    for (int i = 0; i < num_processes; i++) {
        // Create a mix of short and long processes
        int burst = 0;
        if (i % 3 == 0) {
            burst = 5 + rand() % 5;  // Short: 5-9 time units
        } else if (i % 3 == 1) {
            burst = 10 + rand() % 10;  // Medium: 10-19 time units
        } else {
            burst = 20 + rand() % 20;  // Long: 20-39 time units
        }
        
        init_process(&processes[i], i+1, burst);
    }
    
    // Print process details
    printf("Process List:\n");
    printf("+------+------------+\n");
    printf("| Proc | Burst Time |\n");
    printf("+------+------------+\n");
    for (int i = 0; i < num_processes; i++) {
        printf("| P%-3d | %-10d |\n", processes[i].id, processes[i].burst_time);
    }
    printf("+------+------------+\n\n");
    
    // Run both simulations
    simulate_sqms();
    simulate_mqms();
    
    // Compare results
    printf("Comparison of SQMS vs MQMS:\n");
    printf("===========================\n");
    printf("+------+------------+----------------+----------------+\n");
    printf("| Proc | Burst Time | SQMS Complete  | MQMS Complete  |\n");
    printf("+------+------------+----------------+----------------+\n");
    
    int sqms_total = 0, mqms_total = 0;
    int sqms_completed = 0, mqms_completed = 0;
    
    for (int i = 0; i < num_processes; i++) {
        int sqms_time = processes[i].completion_time;
        
        // Re-run MQMS to get completion times
        simulate_mqms();
        int mqms_time = processes[i].completion_time;
        
        printf("| P%-3d | %-10d | %-14d | %-14d |\n", 
               processes[i].id, processes[i].burst_time, 
               sqms_time, mqms_time);
        
        if (sqms_time > 0) {
            sqms_total += sqms_time;
            sqms_completed++;
        }
        
        if (mqms_time > 0) {
            mqms_total += mqms_time;
            mqms_completed++;
        }
    }
    
    printf("+------+------------+----------------+----------------+\n");
    printf("SQMS Avg Completion: %.2f\n", sqms_completed > 0 ? (float)sqms_total/sqms_completed : 0);
    printf("MQMS Avg Completion: %.2f\n", mqms_completed > 0 ? (float)mqms_total/mqms_completed : 0);
    
    printf("\nKey Observations:\n");
    printf("1. MQMS typically has fewer cache misses due to better cache affinity\n");
    printf("2. SQMS provides better load balancing but with higher contention\n");
    printf("3. Work stealing in MQMS helps balance load while preserving some affinity\n");
    
    return 0;
}