#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(stdout);

    pid = fork();

    if (pid < 0) {
        perror("fork error");
        return EXIT_FAILURE;
    } 
    else if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        _exit(127);
    } 
    else {
        if (waitpid(pid, &status, 0) == (pid_t)-1) {
            perror("waitpid error");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("[PARENT] Command exited normally with code: %d\n", 
                   WEXITSTATUS(status));
        } 
        else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("[PARENT] Command terminated by signal: %d (%s)\n", 
                   sig, strsignal(sig));
        }
    }

    return EXIT_SUCCESS;
}
