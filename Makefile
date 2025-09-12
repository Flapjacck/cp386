# Main Makefile for CP386 Course Materials

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lpthread

# Note 1 targets
NOTE1_CPU_DIR = note1/cpu_virtualization
NOTE1_MEM_DIR = note1/memory_virtualization
NOTE1_THREAD_DIR = note1/threads

NOTE1_TARGETS = $(NOTE1_CPU_DIR)/cpu $(NOTE1_MEM_DIR)/mem $(NOTE1_THREAD_DIR)/thread

# Note 3 targets
NOTE3_PROC_DIR = note3/process_creation
NOTE3_EXEC_DIR = note3/process_execution
NOTE3_IO_DIR = note3/io_redirection
NOTE3_PIPE_DIR = note3/pipes

NOTE3_TARGETS = $(NOTE3_PROC_DIR)/p1 $(NOTE3_PROC_DIR)/p2 \
                $(NOTE3_EXEC_DIR)/p3 $(NOTE3_EXEC_DIR)/exec_example \
                $(NOTE3_IO_DIR)/p4 $(NOTE3_IO_DIR)/redirect_demo \
                $(NOTE3_PIPE_DIR)/pipe_demo $(NOTE3_PIPE_DIR)/advanced_pipes

# All targets
ALL_TARGETS = $(NOTE1_TARGETS) $(NOTE3_TARGETS)

.PHONY: all note1 note3 clean help

# Default target
all: $(ALL_TARGETS)
	@echo "All programs compiled successfully!"

# Note-specific targets
note1: $(NOTE1_TARGETS)
	@echo "Note 1 programs compiled successfully!"

note3: $(NOTE3_TARGETS)
	@echo "Note 3 programs compiled successfully!"

# Note 1 targets
$(NOTE1_CPU_DIR)/cpu: $(NOTE1_CPU_DIR)/cpu.c common.h
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE1_MEM_DIR)/mem: $(NOTE1_MEM_DIR)/mem.c common.h
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE1_THREAD_DIR)/thread: $(NOTE1_THREAD_DIR)/thread.c common.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Note 3 targets
$(NOTE3_PROC_DIR)/p1: $(NOTE3_PROC_DIR)/p1.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_PROC_DIR)/p2: $(NOTE3_PROC_DIR)/p2.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_EXEC_DIR)/p3: $(NOTE3_EXEC_DIR)/p3.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_EXEC_DIR)/exec_example: $(NOTE3_EXEC_DIR)/exec_example.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_IO_DIR)/p4: $(NOTE3_IO_DIR)/p4.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_IO_DIR)/redirect_demo: $(NOTE3_IO_DIR)/redirect_demo.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_PIPE_DIR)/pipe_demo: $(NOTE3_PIPE_DIR)/pipe_demo.c
	$(CC) $(CFLAGS) -o $@ $<

$(NOTE3_PIPE_DIR)/advanced_pipes: $(NOTE3_PIPE_DIR)/advanced_pipes.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean target
clean:
	@echo "Cleaning build files..."
	@rm -f $(ALL_TARGETS)
	@rm -f note3/io_redirection/*.output
	@rm -f note3/io_redirection/input.txt
	@echo "Clean complete!"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build all programs"
	@echo "  note1   - Build Note 1 programs only"
	@echo "  note3   - Build Note 3 programs only"
	@echo "  clean   - Remove all compiled programs and output files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Note 1 programs:"
	@echo "  - note1/cpu_virtualization/cpu"
	@echo "  - note1/memory_virtualization/mem"
	@echo "  - note1/threads/thread"
	@echo ""
	@echo "Note 3 programs:"
	@echo "  - note3/process_creation/p1, p2"
	@echo "  - note3/process_execution/p3, exec_example"
	@echo "  - note3/io_redirection/p4, redirect_demo"
	@echo "  - note3/pipes/pipe_demo, advanced_pipes"
