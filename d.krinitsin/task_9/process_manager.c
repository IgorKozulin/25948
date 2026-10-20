#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting program. My PID: %d\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* --- Дочерний процесс --- */
        printf("[CHILD] I am the child process. My PID: %d\n", getpid());
        execlp("cat", "cat", argv[1], NULL);

        /* Сюда попадаем только если execlp упал */
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        /* --- Родительский процесс --- */
        printf("[PARENT] I am the parent. Child PID: %d\n", pid);
        printf("[PARENT] Printing some text while child is working...\n");

        int status;
        printf("[PARENT] Waiting for child to finish...\n");
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("[PARENT] Child exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("[PARENT] Child terminated abnormally\n");
        }

        printf("[PARENT] Child has finished. This is the last line printed by parent.\n");
    }

    return 0;
}
