#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
    (void)argc; (void)argv; // Suppress unused parameter warnings
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) { 
        // child: redirect standard output to a file
        close(STDOUT_FILENO);
        open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        
        char *myargs[3];
        myargs[0] = "wc";
        myargs[1] = "p4.c";
        myargs[2] = NULL;
        execvp(myargs[0], myargs);
    } else {
        wait(NULL);
        printf("Child process completed. Check p4.output file.\n");
    }
    return 0;
}
