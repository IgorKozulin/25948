#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Program start. My PID: %ld\n", (long)getpid());
    fflush(stdout);

    pid = fork();

    if (pid < 0) {
        perror("fork error");
        return EXIT_FAILURE;
    } 
    else if (pid == 0) {
        /* Child process */
        printf("[CHILD] Process started. My PID: %ld, Parent PID: %ld\n", 
               (long)getpid(), (long)getppid());
        fflush(stdout);

        execlp("cat", "cat", argv[1], (char *)NULL);

        perror("[CHILD] execlp error");
        _exit(EXIT_FAILURE);
    } 
    else {
        /* Parent process */
        printf("[PARENT] Created child with PID: %ld\n", (long)pid);
        printf("[PARENT] Printing text while child is working...\n");
        fflush(stdout);

        if (waitpid(pid, &status, 0) == (pid_t)-1) {
            perror("[PARENT] waitpid error");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("[PARENT] Child finished with exit status: %d\n", 
                   WEXITSTATUS(status));
        }

        printf("[PARENT] Child has finished. This is the last line printed by parent.\n");
    }

    return EXIT_SUCCESS;
}
