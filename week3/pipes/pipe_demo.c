#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    int p[2];  // pipe file descriptors
    char *argv_wc[2];
    
    argv_wc[0] = "wc";
    argv_wc[1] = NULL;

    if (pipe(p) < 0) {
        fprintf(stderr, "pipe failed\n");
        exit(1);
    }
    
    if (fork() == 0) {
        // Child process (will become wc)
        close(0);       // close stdin
        dup(p[0]);      // make stdin = read end of pipe
        close(p[0]);    // close original read end
        close(p[1]);    // close write end (child doesn't need it)
        
        execvp("wc", argv_wc);
        fprintf(stderr, "exec failed\n");
        exit(1);
    } else {
        // Parent process (writer)
        close(p[0]);    // close read end (parent doesn't need it)
        write(p[1], "hello world\n", 12);
        write(p[1], "this is a test\n", 15);
        write(p[1], "of the pipe system\n", 19);
        close(p[1]);    // close write end (signals EOF to child)
        
        wait(NULL);     // wait for child to complete
    }
    
    return 0;
}
