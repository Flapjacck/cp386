# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/*
 * schedule_rr.c - Round Robin Scheduler Implementation
 * 
 * This program demonstrates the Round Robin scheduling algorithm
 * with a simulation of process execution.
 */

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int first_run_time;  // For response time calculation
} Process;

void round_robin(Process *processes, int n, int quantum) {
    int completed = 0;
    int current_time = 0;
    
    // Initialize remaining time and first_run_time
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_run_time = -1;  // Not started yet
    }
    
    printf("Execution Timeline:\n");
    
    while (completed < n) {
        int idle = 1;
        
        for (int i = 0; i < n; i++) {
            // Skip if process hasn't arrived yet or is already completed
            if (processes[i].arrival_time > current_time || processes[i].remaining_time == 0) {
                continue;
            }
            
            // This is the first time this process runs
            if (processes[i].first_run_time == -1) {
                processes[i].first_run_time = current_time;
            }
            
            idle = 0;  // System is not idle
            
            // Calculate execution time in this round
            int execution_time = (processes[i].remaining_time < quantum) ? 
                                 processes[i].remaining_time : quantum;
            
            printf("Time %d-%d: Process %d runs\n", 
                   current_time, current_time + execution_time, processes[i].id);
            
            // Update remaining time
            processes[i].remaining_time -= execution_time;
            current_time += execution_time;
            
            // If process completes
            if (processes[i].remaining_time == 0) {
                completed++;
                processes[i].completion_time = current_time;
                processes[i].turnaround_time = processes[i].completion_time - 
                                               processes[i].arrival_time;
                processes[i].waiting_time = processes[i].turnaround_time - 
                                            processes[i].burst_time;
                
                printf("Time %d: Process %d completes\n", current_time, processes[i].id);
            }
        }
        
        // If system was idle, advance time to next process arrival
        if (idle) {
            int next_arrival = INT_MAX;
            for (int i = 0; i < n; i++) {
                if (processes[i].arrival_time > current_time && 
                    processes[i].arrival_time < next_arrival) {
                    next_arrival = processes[i].arrival_time;
                }
            }
            
            if (next_arrival != INT_MAX) {
                printf("Time %d-%d: CPU idle\n", current_time, next_arrival);
                current_time = next_arrival;
            } else {
                // All processes have arrived but not all completed (should not happen)
                printf("Error: No runnable process but not all completed\n");
                break;
            }
        }
    }
}

void print_results(Process *processes, int n) {
    printf("\n");
    printf("+------+-------------+------------+----------------+----------------+-------------+-------------+\n");
    printf("| Proc | Arrival     | CPU Burst  | Completion     | Turnaround     | Waiting     | Response    |\n");
    printf("+------+-------------+------------+----------------+----------------+-------------+-------------+\n");
    
    float avg_turnaround = 0, avg_waiting = 0, avg_response = 0;
    
    for (int i = 0; i < n; i++) {
        int response_time = processes[i].first_run_time - processes[i].arrival_time;
        
        printf("| P%-3d | %-11d | %-10d | %-14d | %-14d | %-11d | %-11d |\n",
               processes[i].id,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               response_time);
        
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
        avg_response += response_time;
    }
    
    avg_turnaround /= n;
    avg_waiting /= n;
    avg_response /= n;
    
    printf("+------+-------------+------------+----------------+----------------+-------------+-------------+\n");
    printf("Average Turnaround Time: %.2f\n", avg_turnaround);
    printf("Average Waiting Time: %.2f\n", avg_waiting);
    printf("Average Response Time: %.2f\n", avg_response);
}

int main() {
    // Sample process data for Round Robin demonstration
    Process processes[] = {
        {1, 0, 24, 0, 0, 0, 0, -1},  // Process 1: arrives at 0, needs 24 time units
        {2, 0, 3, 0, 0, 0, 0, -1},   // Process 2: arrives at 0, needs 3 time units
        {3, 0, 3, 0, 0, 0, 0, -1}    // Process 3: arrives at 0, needs 3 time units
    };
    
    int n = sizeof(processes) / sizeof(processes[0]);
    int quantum = 5;  // Time quantum for RR
    
    printf("Round Robin (RR) Scheduling Algorithm Demo\n\n");
    printf("Process sequence: P1 (24ms), P2 (3ms), P3 (3ms)\n");
    printf("Time Quantum: %d time units\n\n", quantum);
    
    // Run the scheduler
    round_robin(processes, n, quantum);
    
    // Display results
    print_results(processes, n);
    
    printf("\nRound Robin provides better response time but worse turnaround time than FCFS\n");
    
    return 0;
}