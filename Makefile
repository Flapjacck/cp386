# Main Makefile for CP386 Course Materials

CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lpthread

# Week 1 targets
WEEK1_CPU_DIR = week1/cpu_virtualization
WEEK1_MEM_DIR = week1/memory_virtualization
WEEK1_THREAD_DIR = week1/threads

WEEK1_TARGETS = $(WEEK1_CPU_DIR)/cpu $(WEEK1_MEM_DIR)/mem $(WEEK1_THREAD_DIR)/thread

# Week 3 targets
WEEK3_PROC_DIR = week3/process_creation
WEEK3_EXEC_DIR = week3/process_execution
WEEK3_IO_DIR = week3/io_redirection
WEEK3_PIPE_DIR = week3/pipes

WEEK3_TARGETS = $(WEEK3_PROC_DIR)/p1 $(WEEK3_PROC_DIR)/p2 \
                $(WEEK3_EXEC_DIR)/p3 $(WEEK3_EXEC_DIR)/exec_example \
                $(WEEK3_IO_DIR)/p4 $(WEEK3_IO_DIR)/redirect_demo \
                $(WEEK3_PIPE_DIR)/pipe_demo $(WEEK3_PIPE_DIR)/advanced_pipes

# All targets
ALL_TARGETS = $(WEEK1_TARGETS) $(WEEK3_TARGETS)

.PHONY: all week1 week3 clean help

# Default target
all: $(ALL_TARGETS)
	@echo "All programs compiled successfully!"

# Week-specific targets
week1: $(WEEK1_TARGETS)
	@echo "Week 1 programs compiled successfully!"

week3: $(WEEK3_TARGETS)
	@echo "Week 3 programs compiled successfully!"

# Week 1 targets
$(WEEK1_CPU_DIR)/cpu: $(WEEK1_CPU_DIR)/cpu.c common.h
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK1_MEM_DIR)/mem: $(WEEK1_MEM_DIR)/mem.c common.h
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK1_THREAD_DIR)/thread: $(WEEK1_THREAD_DIR)/thread.c common.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

# Week 3 targets
$(WEEK3_PROC_DIR)/p1: $(WEEK3_PROC_DIR)/p1.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_PROC_DIR)/p2: $(WEEK3_PROC_DIR)/p2.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_EXEC_DIR)/p3: $(WEEK3_EXEC_DIR)/p3.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_EXEC_DIR)/exec_example: $(WEEK3_EXEC_DIR)/exec_example.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_IO_DIR)/p4: $(WEEK3_IO_DIR)/p4.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_IO_DIR)/redirect_demo: $(WEEK3_IO_DIR)/redirect_demo.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_PIPE_DIR)/pipe_demo: $(WEEK3_PIPE_DIR)/pipe_demo.c
	$(CC) $(CFLAGS) -o $@ $<

$(WEEK3_PIPE_DIR)/advanced_pipes: $(WEEK3_PIPE_DIR)/advanced_pipes.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean target
clean:
	@echo "Cleaning build files..."
	@rm -f $(ALL_TARGETS)
	@rm -f week3/io_redirection/*.output
	@rm -f week3/io_redirection/input.txt
	@echo "Clean complete!"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build all programs"
	@echo "  week1   - Build Week 1 programs only"
	@echo "  week3   - Build Week 3 programs only"
	@echo "  clean   - Remove all compiled programs and output files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Week 1 programs:"
	@echo "  - week1/cpu_virtualization/cpu"
	@echo "  - week1/memory_virtualization/mem"
	@echo "  - week1/threads/thread"
	@echo ""
	@echo "Week 3 programs:"
	@echo "  - week3/process_creation/p1, p2"
	@echo "  - week3/process_execution/p3, exec_example"
	@echo "  - week3/io_redirection/p4, redirect_demo"
	@echo "  - week3/pipes/pipe_demo, advanced_pipes"
