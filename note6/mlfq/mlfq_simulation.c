# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

/*
 * mlfq_simulation.c - Multi-Level Feedback Queue Scheduler Simulation
 * 
 * This program demonstrates the MLFQ scheduling algorithm with a simulation
 * of process execution, showing how the scheduler adapts to different process types.
 */

#define MAX_PROCESSES 10
#define NUM_QUEUES 3

typedef struct {
    int id;                 // Process ID
    int arrival_time;       // Time when process arrives
    int burst_time;         // Total CPU time needed
    int remaining_time;     // Remaining CPU time
    int current_queue;      // Current priority queue (0 = highest, NUM_QUEUES-1 = lowest)
    int time_in_current_quantum; // Time used in current quantum
    int completion_time;    // When process finishes
    int turnaround_time;    // Completion time - arrival time
    int waiting_time;       // Turnaround time - burst time
    int first_run_time;     // Time of first execution (for response time)
    int is_io_bound;        // 1 if IO bound, 0 if CPU bound
} Process;

typedef struct {
    Process *processes[MAX_PROCESSES];
    int count;
    int time_quantum;       // Time quantum for this queue
} Queue;

Queue queues[NUM_QUEUES];
int current_time = 0;
int boost_interval = 50;    // Priority boost interval
int last_boost_time = 0;

void init_queues() {
    for (int i = 0; i < NUM_QUEUES; i++) {
        queues[i].count = 0;
        // Time quantum increases with lower priority
        queues[i].time_quantum = (1 << i) * 10; // 10, 20, 40, ...
    }
}

void add_process_to_queue(Process *p, int queue_level) {
    if (queues[queue_level].count < MAX_PROCESSES) {
        queues[queue_level].processes[queues[queue_level].count++] = p;
        p->current_queue = queue_level;
        p->time_in_current_quantum = 0;
    }
}

Process* get_next_process() {
    // Priority boost if needed (Rule 5)
    if (current_time - last_boost_time >= boost_interval) {
        printf("Time %d: Priority boost!\n", current_time);
        // Move all processes to highest priority queue
        for (int q = 1; q < NUM_QUEUES; q++) {
            for (int i = 0; i < queues[q].count; i++) {
                add_process_to_queue(queues[q].processes[i], 0);
            }
            queues[q].count = 0;
        }
        last_boost_time = current_time;
    }
    
    // Find highest priority non-empty queue (Rule 1)
    for (int q = 0; q < NUM_QUEUES; q++) {
        if (queues[q].count > 0) {
            // Get first process in queue (round-robin within priority level - Rule 2)
            Process *p = queues[q].processes[0];
            
            // Shift remaining processes left
            for (int i = 0; i < queues[q].count - 1; i++) {
                queues[q].processes[i] = queues[q].processes[i+1];
            }
            
            queues[q].count--;
            return p;
        }
    }
    
    return NULL; // No process available
}

void run_mlfq_simulation(Process *processes, int n) {
    int completed = 0;
    
    // Initialize queues
    init_queues();
    
    // Set up initial process state
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].current_queue = 0;
        processes[i].time_in_current_quantum = 0;
        processes[i].first_run_time = -1;
    }
    
    printf("\nMLFQ Simulation Start\n");
    printf("=====================\n\n");
    
    while (completed < n) {
        // Check for new arrivals
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == current_time) {
                printf("Time %d: Process %d arrives (burst=%d, type=%s)\n", 
                       current_time, processes[i].id, processes[i].burst_time, 
                       processes[i].is_io_bound ? "I/O-bound" : "CPU-bound");
                // Rule 3: New processes start at highest priority
                add_process_to_queue(&processes[i], 0);
            }
        }
        
        // Get next process to run
        Process *current_proc = get_next_process();
        
        if (current_proc != NULL) {
            // Record first run time if not set
            if (current_proc->first_run_time == -1) {
                current_proc->first_run_time = current_time;
            }
            
            int q = current_proc->current_queue;
            int time_slice = queues[q].time_quantum;
            int run_time;
            
            // For I/O bound processes, they use only part of their quantum before yielding
            if (current_proc->is_io_bound) {
                // I/O bound processes use ~20% of their quantum then yield for I/O
                run_time = time_slice / 5;
                if (run_time > current_proc->remaining_time) {
                    run_time = current_proc->remaining_time;
                }
            } else {
                // CPU bound processes use their full quantum unless they complete
                run_time = (current_proc->remaining_time < time_slice - current_proc->time_in_current_quantum) ? 
                           current_proc->remaining_time : 
                           time_slice - current_proc->time_in_current_quantum;
            }
            
            // Run the process
            printf("Time %d: Running Process %d (priority=%d, remaining=%d, quantum=%d)\n",
                   current_time, current_proc->id, q, current_proc->remaining_time, time_slice);
            
            current_time += run_time;
            current_proc->remaining_time -= run_time;
            current_proc->time_in_current_quantum += run_time;
            
            // Process completed
            if (current_proc->remaining_time == 0) {
                printf("Time %d: Process %d completed\n", current_time, current_proc->id);
                current_proc->completion_time = current_time;
                current_proc->turnaround_time = current_proc->completion_time - 
                                               current_proc->arrival_time;
                current_proc->waiting_time = current_proc->turnaround_time - 
                                            current_proc->burst_time;
                completed++;
            }
            // Process yielded for I/O (Rule 4b)
            else if (current_proc->is_io_bound && run_time < time_slice) {
                printf("Time %d: Process %d yields for I/O (keeps priority=%d)\n", 
                       current_time, current_proc->id, q);
                
                // Rule 4b: If process doesn't use full time slice due to I/O, it stays at same priority
                // Simulate I/O time (will return after a delay)
                int io_time = 10; // Fixed I/O time for simulation
                current_proc->time_in_current_quantum = 0; // Reset time in quantum
                current_proc->arrival_time = current_time + io_time; // Will "re-arrive" after I/O
                
            }
            // Process used its full quantum (Rule 4a)
            else if (current_proc->time_in_current_quantum >= time_slice) {
                // Rule 4a: If process uses full quantum, decrease its priority
                int next_queue = (q < NUM_QUEUES - 1) ? q + 1 : q;
                
                printf("Time %d: Process %d used full quantum, demoted to priority=%d\n", 
                       current_time, current_proc->id, next_queue);
                
                current_proc->time_in_current_quantum = 0;
                add_process_to_queue(current_proc, next_queue);
            }
            // Process still has quantum remaining
            else {
                printf("Time %d: Process %d returned to queue (priority=%d)\n", 
                       current_time, current_proc->id, q);
                add_process_to_queue(current_proc, q);
            }
        }
        // No process available to run
        else {
            printf("Time %d: CPU idle\n", current_time);
            
            // Find next arrival time
            int next_arrival = -1;
            for (int i = 0; i < n; i++) {
                if (processes[i].remaining_time > 0 && 
                    processes[i].arrival_time > current_time) {
                    if (next_arrival == -1 || processes[i].arrival_time < next_arrival) {
                        next_arrival = processes[i].arrival_time;
                    }
                }
            }
            
            if (next_arrival != -1) {
                current_time = next_arrival;
            } else {
                // Should not happen unless there's a bug
                printf("Error: No process to run but not all completed\n");
                break;
            }
        }
    }
}

void print_results(Process *processes, int n) {
    printf("\nResults:\n");
    printf("+------+-------------+----------+-------------+------------+----------------+------------+\n");
    printf("| Proc | Type        | Burst    | Response    | Completion | Turnaround     | Waiting    |\n");
    printf("+------+-------------+----------+-------------+------------+----------------+------------+\n");
    
    float avg_turnaround = 0, avg_waiting = 0, avg_response = 0;
    float avg_turnaround_io = 0, avg_response_io = 0;
    float avg_turnaround_cpu = 0, avg_response_cpu = 0;
    int n_io = 0, n_cpu = 0;
    
    for (int i = 0; i < n; i++) {
        int response_time = processes[i].first_run_time - processes[i].arrival_time;
        
        printf("| P%-3d | %-11s | %-8d | %-11d | %-10d | %-14d | %-10d |\n",
               processes[i].id,
               processes[i].is_io_bound ? "I/O-bound" : "CPU-bound",
               processes[i].burst_time,
               response_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
        
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
        avg_response += response_time;
        
        if (processes[i].is_io_bound) {
            avg_turnaround_io += processes[i].turnaround_time;
            avg_response_io += response_time;
            n_io++;
        } else {
            avg_turnaround_cpu += processes[i].turnaround_time;
            avg_response_cpu += response_time;
            n_cpu++;
        }
    }
    
    avg_turnaround /= n;
    avg_waiting /= n;
    avg_response /= n;
    
    if (n_io > 0) {
        avg_turnaround_io /= n_io;
        avg_response_io /= n_io;
    }
    
    if (n_cpu > 0) {
        avg_turnaround_cpu /= n_cpu;
        avg_response_cpu /= n_cpu;
    }
    
    printf("+------+-------------+----------+-------------+------------+----------------+------------+\n");
    printf("Overall Average Turnaround Time: %.2f\n", avg_turnaround);
    printf("Overall Average Waiting Time: %.2f\n", avg_waiting);
    printf("Overall Average Response Time: %.2f\n\n", avg_response);
    
    printf("I/O-bound Average Turnaround Time: %.2f\n", avg_turnaround_io);
    printf("I/O-bound Average Response Time: %.2f\n\n", avg_response_io);
    
    printf("CPU-bound Average Turnaround Time: %.2f\n", avg_turnaround_cpu);
    printf("CPU-bound Average Response Time: %.2f\n", avg_response_cpu);
}

int main() {
    // Sample processes for MLFQ demonstration
    Process processes[] = {
        // id, arrival, burst, remaining, queue, quantum_time, completion, turnaround, waiting, first_run, io_bound
        {1, 0, 100, 0, 0, 0, 0, 0, 0, -1, 0},  // Long CPU-bound process
        {2, 0, 5, 0, 0, 0, 0, 0, 0, -1, 1},    // Short I/O-bound process
        {3, 0, 5, 0, 0, 0, 0, 0, 0, -1, 1},    // Short I/O-bound process
        {4, 10, 80, 0, 0, 0, 0, 0, 0, -1, 0},  // Another CPU-bound process
        {5, 20, 15, 0, 0, 0, 0, 0, 0, -1, 1}   // Medium I/O-bound process
    };
    
    int n = sizeof(processes) / sizeof(processes[0]);
    
    printf("Multi-Level Feedback Queue (MLFQ) Scheduling Algorithm Demo\n");
    printf("=========================================================\n\n");
    printf("MLFQ Rules:\n");
    printf("1. If Priority(A) > Priority(B), A runs\n");
    printf("2. If Priority(A) = Priority(B), A & B run in round-robin\n");
    printf("3. New job starts at highest priority\n");
    printf("4a. If job uses full time slice, it moves down one queue\n");
    printf("4b. If job gives up CPU before time slice is used, it stays at same priority\n");
    printf("5. After some time period S, move all jobs to highest priority queue\n\n");
    
    printf("Queue Configuration:\n");
    printf("Queue 0 (highest): Time Quantum = 10\n");
    printf("Queue 1: Time Quantum = 20\n");
    printf("Queue 2 (lowest): Time Quantum = 40\n");
    printf("Priority Boost Interval: %d time units\n", boost_interval);
    
    run_mlfq_simulation(processes, n);
    
    print_results(processes, n);
    
    printf("\nObservations:\n");
    printf("1. I/O-bound processes maintain higher priority by yielding before using full quantum\n");
    printf("2. CPU-bound processes get demoted to lower queues after using full quanta\n");
    printf("3. Priority boost prevents starvation of lower-priority processes\n");
    printf("4. I/O-bound processes have better response time than CPU-bound processes\n");
    
    return 0;
}