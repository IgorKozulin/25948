#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("[CHILD] Child process started\n");

        execlp("cat", "cat", argv[1], NULL);

        perror("execlp");
        exit(EXIT_FAILURE);
    }
    else {
        printf("[PARENT] Parent process is working\n");

        int status;

        waitpid(pid, &status, 0);

        printf("[PARENT] Child has finished\n");
        printf("[PARENT] This is the last line\n");
    }

    return 0;
}
