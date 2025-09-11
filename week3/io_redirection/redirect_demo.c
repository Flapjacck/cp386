#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

// Demonstrates simple file descriptor behavior
void fd_demo() {
    printf("File descriptor demo:\n");
    
    if (fork() == 0) {
        write(1, "hello ", 6);
        exit(0);
    } else {
        wait(NULL);
        write(1, "world\n", 6);
    }
}

// Demonstrates input redirection
void input_redirect_demo() {
    printf("\nInput redirection demo:\n");
    
    // Create a test input file
    int fd = open("input.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if (fd >= 0) {
        write(fd, "Hello from input file!\nLine 2\nLine 3\n", 37);
        close(fd);
    }
    
    char *argv[2];
    argv[0] = "cat";
    argv[1] = NULL;
    
    if (fork() == 0) {
        close(0);  // close stdin
        open("input.txt", O_RDONLY);  // this becomes new stdin (fd 0)
        execvp("cat", argv);
    } else {
        wait(NULL);
    }
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    fd_demo();
    input_redirect_demo();
    
    printf("Demo completed. Check input.txt file.\n");
    return 0;
}
