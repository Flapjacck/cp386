# I/O Redirection

## Overview

These programs demonstrate how to redirect standard input, output, and error streams in Unix-like systems. This is the foundation of how shells implement commands like `command > file` and `command < file`.

## Key Concepts

- **File Descriptors**: Integer handles for open files (0=stdin, 1=stdout, 2=stderr)
- **Standard Streams**: stdin (0), stdout (1), stderr (2)
- **Redirection**: Changing where input comes from or output goes to
- **File Operations**: open(), close(), read(), write()
- **File Modes**: O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC

## Programs

### p4.c - Output Redirection

Demonstrates redirecting stdout to a file (equivalent to `wc p4.c > p4.output`).

### redirect_demo.c - Complete I/O Redirection Demo

Shows both input and output redirection with practical examples.

## How to Compile and Run

### Compile

```bash
gcc -o p4 p4.c
gcc -o redirect_demo redirect_demo.c
```

### Run p4 (output redirection)

```bash
./p4
cat p4.output  # View the redirected output
```

### Run redirect_demo

```bash
./redirect_demo
cat input.txt  # View the created input file
```

### Compare with shell redirection

```bash
# Shell equivalent of p4.c
wc p4.c > shell_output.txt
diff p4.output shell_output.txt  # Should be identical
```

## Expected Output

### p4.c

```bash
Child process completed. Check p4.output file.
```

And `p4.output` contains:

```bash
      24      85     578 p4.c
```

### redirect_demo.c

```bash
File descriptor demo:
hello world

Input redirection demo:
Hello from input file!
Line 2
Line 3
Demo completed. Check input.txt file.
```

## How Redirection Works

1. **Close** the target file descriptor (0, 1, or 2)
2. **Open** a new file - it gets the lowest available FD number
3. **Execute** the program - it uses the new file without knowing

## File Descriptor Mechanics

```c
close(1);                           // Close stdout
open("file.txt", O_WRONLY);         // This becomes new fd 1
printf("Hello");                    // Goes to file.txt instead of terminal
```

## Common Redirection Patterns

### Output Redirection (>)

```c
close(1);
open("output.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
```

### Append Redirection (>>)

```c
close(1);
open("output.txt", O_CREAT|O_WRONLY|O_APPEND, 0644);
```

### Input Redirection (<)

```c
close(0);
open("input.txt", O_RDONLY);
```

### Error Redirection (2>)

```c
close(2);
open("error.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
```

## Learning Points

1. File descriptors are just integers that refer to open files
2. Programs read from fd 0 and write to fd 1 by default
3. Closing and reopening changes where I/O goes
4. This mechanism enables powerful shell features
5. All Unix I/O is based on this simple concept

## Additional Commands for I/O Exploration

```bash
# View file contents
cat filename

# Redirect output to file
command > file

# Append output to file
command >> file

# Redirect input from file
command < file

# Redirect both input and output
command < input_file > output_file

# Redirect stderr
command 2> error_file

# Redirect stdout and stderr
command > output_file 2>&1

# View file descriptor information
ls -la /proc/PID/fd/

# Monitor file operations
strace -e open,close,read,write ./program
```

## Practical Examples

```bash
# Count lines in multiple files, save to result
wc -l *.c > line_counts.txt

# Sort a file and save result
sort < unsorted.txt > sorted.txt

# Search and save results
grep "printf" *.c > printf_usage.txt

# Combine commands with redirection
cat file1.txt file2.txt > combined.txt
```

## Experiment Suggestions

1. Try redirecting stderr (fd 2) to a file
2. Implement your own version of `tee` command
3. Create a program that reads from stdin and writes to multiple files
4. Explore what happens with invalid file permissions
