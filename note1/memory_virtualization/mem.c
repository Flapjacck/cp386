/*
 * ===================================================================
 * Memory Virtualization Demonstration Program
 * ===================================================================
 * 
 * This program demonstrates memory virtualization - how each process
 * sees its own private virtual address space, even though multiple
 * processes share the same physical memory. This is achieved through
 * the Memory Management Unit (MMU) and operating system cooperation.
 * 
 * CORE CONCEPTS DEMONSTRATED:
 * ---------------------------
 * 1. Virtual Address Space: Each process has its own view of memory
 * 2. Address Translation: Virtual addresses mapped to physical addresses
 * 3. Memory Isolation: Processes cannot access each other's memory
 * 4. Virtual Memory Illusion: Processes see a flat, contiguous address space
 * 
 * MEMORY MANAGEMENT UNIT (MMU):
 * ----------------------------
 * The MMU is hardware that translates virtual addresses to physical addresses:
 * 
 * Virtual Address → [MMU + Page Tables] → Physical Address
 * 
 * Key Components:
 * - Page Tables: OS-managed data structures mapping virtual to physical pages
 * - Translation Lookaside Buffer (TLB): Hardware cache for address translations
 * - Page Fault Handler: OS code that handles memory access violations
 * 
 * ADDRESS SPACE LAYOUT:
 * --------------------
 * In a typical process address space:
 * 
 * 0xFFFFFFFF ┌─────────────────┐
 *            │   Kernel Space  │ ← OS kernel memory (protected)
 * 0xC0000000 ├─────────────────┤
 *            │      Stack      │ ← Local variables, function calls
 *            │        ↓        │   (grows downward)
 *            │                 │
 *            │   Free Space    │
 *            │                 │
 *            │        ↑        │
 *            │      Heap       │ ← malloc() allocates here (grows upward)
 *            ├─────────────────┤
 *            │      Data       │ ← Global/static variables
 *            ├─────────────────┤
 *            │      Text       │ ← Program code (instructions)
 * 0x00000000 └─────────────────┘
 * 
 * References:
 * - OSDev Wiki: https://wiki.osdev.org/Paging
 * - Intel SDM Volume 3: System Programming Guide
 * - OSTEP Chapter 13: Address Spaces
 */

#include <unistd.h>     // For getpid() - process identification
#include <stdio.h>      // For printf(), fprintf()
#include <stdlib.h>     // For malloc(), exit()
#include <stdint.h>     // For uintptr_t - integer type that can hold pointer
#include <string.h>     // For strncmp() - string comparison
#include <sys/mman.h>   // For mmap() - memory mapping
#include "../../common.h"  // Our common utility functions

/*
 * DEMONSTRATION STRATEGY:
 * ======================
 * 
 * This program will:
 * 1. Allocate memory on the heap using malloc()
 * 2. Display the virtual address of the allocated memory
 * 3. Continuously modify the memory and show the value
 * 4. Include process ID to distinguish between multiple instances
 * 
 * When run with multiple instances, you'll observe:
 * - Same virtual addresses may be reused across processes
 * - Each process has its own private copy of memory
 * - No process can interfere with another's memory
 */

// Function to print detailed memory information
void print_memory_info(void *addr, const char *description) {
    uintptr_t virtual_addr = (uintptr_t)addr;
    
    printf("  %s:\n", description);
    printf("    Virtual Address: %p (0x%lx)\n", addr, virtual_addr);
    printf("    Address Space Region: ");
    
    // Analyze which memory region this address belongs to
    if (virtual_addr < 0x400000) {
        printf("Low Memory (likely NULL page protection)\n");
    } else if (virtual_addr < 0x600000) {
        printf("Text Segment (program code)\n");
    } else if (virtual_addr < 0x800000) {
        printf("Data Segment (global/static variables)\n");
    } else if (virtual_addr < 0x40000000) {
        printf("Heap Region (malloc allocations)\n");
    } else if (virtual_addr < 0x80000000) {
        printf("Memory Mapped Files/Libraries\n");
    } else if (virtual_addr >= 0x7fff0000) {
        printf("Stack Region (local variables)\n");
    } else {
        printf("Unknown Region\n");
    }
    printf("\n");
}

// Function to demonstrate memory protection
void demonstrate_memory_protection() {
    printf("=== Memory Protection Demonstration ===\n");
    
    // Try to access a NULL pointer (should be protected)
    printf("Attempting to access NULL pointer...\n");
    printf("(This would cause segmentation fault in real scenario)\n");
    printf("NULL pointer address: %p\n", (void*)NULL);
    
    // Note: We don't actually dereference NULL to avoid crashing the demo
    // In a real scenario: *((int*)NULL) = 42; would cause SIGSEGV
    
    printf("\n");
}

// Function to show virtual memory statistics
void show_memory_stats(pid_t pid) {
    char filename[256];
    FILE *file;
    
    // Try to read memory statistics from /proc (Linux-specific)
    snprintf(filename, sizeof(filename), "/proc/%d/status", pid);
    file = fopen(filename, "r");
    
    if (file) {
        char line[256];
        printf("=== Virtual Memory Statistics (from /proc) ===\n");
        
        while (fgets(line, sizeof(line), file)) {
            // Look for memory-related lines
            if (strncmp(line, "VmSize:", 7) == 0 ||
                strncmp(line, "VmRSS:", 6) == 0 ||
                strncmp(line, "VmData:", 7) == 0 ||
                strncmp(line, "VmStk:", 6) == 0 ||
                strncmp(line, "VmExe:", 6) == 0) {
                printf("  %s", line);
            }
        }
        fclose(file);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    // Suppress unused parameter warnings
    (void)argc; 
    (void)argv;
    
    // Get process identification
    pid_t pid = getpid();
    
    printf("=============================================================\n");
    printf("Memory Virtualization Demonstration - Process %d\n", pid);
    printf("=============================================================\n\n");
    
    /*
     * HEAP MEMORY ALLOCATION
     * =====================
     * 
     * malloc() requests memory from the heap. The OS:
     * 1. Assigns virtual pages to the process
     * 2. Maps them to physical pages via page tables
     * 3. Updates MMU mappings
     * 4. Returns virtual address to the process
     * 
     * Key Point: The returned address is VIRTUAL, not physical.
     * Multiple processes can receive the same virtual address
     * but they map to different physical memory locations.
     */
    printf("=== Step 1: Heap Memory Allocation ===\n");
    
    // Allocate integer on heap
    int *heap_ptr = malloc(sizeof(int));
    if (heap_ptr == NULL) {
        fprintf(stderr, "ERROR: malloc() failed - out of memory\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Successfully allocated %zu bytes on heap\n", sizeof(int));
    print_memory_info(heap_ptr, "Heap-allocated integer");
    
    /*
     * STACK MEMORY EXAMINATION
     * =======================
     * 
     * Local variables are allocated on the stack. Compare
     * stack addresses to heap addresses to see different
     * memory regions in the virtual address space.
     */
    printf("=== Step 2: Stack Memory Examination ===\n");
    
    int stack_var = 42;  // Local variable on stack
    print_memory_info(&stack_var, "Stack-allocated integer");
    
    /*
     * CODE SEGMENT EXAMINATION
     * =======================
     * 
     * Function pointers show us where code is loaded
     * in the virtual address space (text segment).
     */
    printf("=== Step 3: Code Segment Examination ===\n");
    print_memory_info((void*)main, "Main function (code segment)");
    print_memory_info((void*)printf, "printf function (shared library)");
    
    /*
     * VIRTUAL ADDRESS ANALYSIS
     * =======================
     * 
     * Show the relationship between different memory regions
     * and explain what this tells us about virtual memory layout.
     */
    printf("=== Step 4: Virtual Address Space Analysis ===\n");
    
    uintptr_t heap_addr = (uintptr_t)heap_ptr;
    uintptr_t stack_addr = (uintptr_t)&stack_var;
    uintptr_t code_addr = (uintptr_t)main;
    
    printf("Address Comparison:\n");
    printf("  Code  (0x%08lx) < Heap  (0x%08lx) < Stack (0x%08lx)\n", 
           code_addr, heap_addr, stack_addr);
    printf("  This shows the classic virtual memory layout:\n");
    printf("  Low addresses → Code → Data → Heap ↑ ... ↓ Stack ← High addresses\n\n");
    
    // Show memory protection demonstration
    demonstrate_memory_protection();
    
    // Show system memory statistics if available
    show_memory_stats(pid);
    
    /*
     * CONTINUOUS MEMORY UPDATES
     * ========================
     * 
     * Initialize heap memory and continuously update it.
     * This demonstrates:
     * 1. Memory persistence across time
     * 2. Private memory per process
     * 3. Virtual address consistency
     */
    printf("=== Step 5: Continuous Memory Updates ===\n");
    printf("Starting infinite loop - memory updates every second\n");
    printf("Virtual address will remain constant: %p\n", (void*)heap_ptr);
    printf("Use Ctrl+C or 'killall mem' to stop\n\n");
    
    // Initialize the memory location
    *heap_ptr = 0;
    
    unsigned long iterations = 0;
    double start_time = GetTime();
    
    while (1) {
        /*
         * MEMORY ACCESS PATTERN
         * ====================
         * 
         * Each iteration:
         * 1. CPU reads current value from virtual address
         * 2. MMU translates virtual → physical address
         * 3. Memory controller fetches from physical RAM
         * 4. CPU increments value
         * 5. CPU writes back to virtual address
         * 6. MMU translates and writes to physical RAM
         * 
         * This entire process is transparent to our program!
         */
        
        // Wait approximately 1 second (simulating work)
        Spin(1);
        
        // Increment the value at our heap location
        (*heap_ptr)++;
        iterations++;
        
        // Display current state with detailed information
        double current_time = GetTime();
        double elapsed = current_time - start_time;
        
        printf("[PID %5d][Iter %6lu][Time %8.2fs] ", pid, iterations, elapsed);
        printf("Value at %p = %d\n", (void*)heap_ptr, *heap_ptr);
        
        // Periodically show additional memory information
        if (iterations % 10 == 0) {
            printf("              └─ After %lu iterations, virtual address unchanged\n", 
                   iterations);
            printf("                 Physical address may have changed (OS decision)\n");
        }
        
        // Flush output for real-time observation
        fflush(stdout);
    }
    
    /*
     * CLEANUP (never reached due to infinite loop)
     * ===========================================
     * 
     * In a real application, we would:
     * 1. Free allocated memory: free(heap_ptr)
     * 2. Close file descriptors
     * 3. Release other resources
     * 
     * The OS will clean up automatically when process terminates.
     */
    free(heap_ptr);  // This line never executes due to infinite loop above
    return EXIT_SUCCESS;
}
