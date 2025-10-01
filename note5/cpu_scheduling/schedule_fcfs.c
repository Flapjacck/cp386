# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

/*
 * schedule_fcfs.c - First-Come-First-Served Scheduler Implementation
 * 
 * This program demonstrates the FCFS scheduling algorithm
 * with a simulation of process execution.
 */

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
} Process;

void calculate_times(Process *processes, int n) {
    // Sort by arrival time (already assumed to be in order for FCFS)
    
    // Set initial time to first process arrival
    int current_time = processes[0].arrival_time;
    
    for (int i = 0; i < n; i++) {
        // If there's a gap in arrivals, move time forward
        if (processes[i].arrival_time > current_time) {
            current_time = processes[i].arrival_time;
        }
        
        // Execute the process
        printf("Time %d: Process %d starts execution\n", current_time, processes[i].id);
        current_time += processes[i].burst_time;
        
        // Record completion time
        processes[i].completion_time = current_time;
        
        // Calculate turnaround time (completion - arrival)
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        
        // Calculate waiting time (turnaround - burst)
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        
        printf("Time %d: Process %d completes\n", current_time, processes[i].id);
    }
}

void print_results(Process *processes, int n) {
    printf("\n");
    printf("+------+-------------+------------+----------------+----------------+-------------+\n");
    printf("| Proc | Arrival     | CPU Burst  | Completion     | Turnaround     | Waiting     |\n");
    printf("+------+-------------+------------+----------------+----------------+-------------+\n");
    
    float avg_turnaround = 0, avg_waiting = 0;
    
    for (int i = 0; i < n; i++) {
        printf("| P%-3d | %-11d | %-10d | %-14d | %-14d | %-11d |\n",
               processes[i].id,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);
        
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
    }
    
    avg_turnaround /= n;
    avg_waiting /= n;
    
    printf("+------+-------------+------------+----------------+----------------+-------------+\n");
    printf("Average Turnaround Time: %.2f\n", avg_turnaround);
    printf("Average Waiting Time: %.2f\n", avg_waiting);
}

int main() {
    // Sample process data for FCFS demonstration
    // Shows the convoy effect with a long process first
    Process processes[] = {
        {1, 0, 24, 0, 0, 0},  // Process 1: arrives at 0, needs 24 time units
        {2, 0, 3, 0, 0, 0},   // Process 2: arrives at 0, needs 3 time units
        {3, 0, 3, 0, 0, 0}    // Process 3: arrives at 0, needs 3 time units
    };
    
    int n = sizeof(processes) / sizeof(processes[0]);
    
    printf("First-Come-First-Served (FCFS) Scheduling Algorithm Demo\n\n");
    printf("Process sequence: P1 (24ms), P2 (3ms), P3 (3ms)\n\n");
    
    // Run the scheduler
    calculate_times(processes, n);
    
    // Display results
    print_results(processes, n);
    
    printf("\nConvoy effect demonstrated: Short processes (P2, P3) wait for long process (P1)\n");
    
    return 0;
}