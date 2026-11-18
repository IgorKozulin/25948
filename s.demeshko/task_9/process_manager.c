/* Задача 9: создание двух процессов (fork + exec + wait) */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <file-to-cat>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Если stdout перенаправлен в файл, он буферизуется полностью.
       Сбрасываем до fork(), чтобы ребёнок не унаследовал недописанный буфер. */
    fflush(NULL);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {                        /* ---------- ребёнок ---------- */
        printf("[CHILD] I am the child process. My PID: %ld\n", (long)getpid());
        fflush(stdout);                    /* иначе строка потеряется при exec */

        execlp("cat", "cat", argv[1], (char *)NULL);

        /* Сюда попадаем только если exec не удался */
        perror("execlp");
        _exit(EXIT_FAILURE);
    }

    /* ---------- родитель ---------- */
    printf("[PARENT] Starting program. My PID: %ld\n", (long)getpid());
    printf("[PARENT] I am the parent. Child PID: %ld\n", (long)pid);
    printf("[PARENT] Printing some text while child is working...\n");

#ifdef NO_WAIT
    /* Часть 1: не ждём — порядок вывода не гарантирован */
    printf("[PARENT] This is the last line printed by parent (no wait).\n");
    return EXIT_SUCCESS;
#else
    /* Часть 2: ждём завершения ребёнка */
    printf("[PARENT] Waiting for child to finish...\n");

    int status = 0;
    pid_t r;
    do {
        r = waitpid(pid, &status, 0);
    } while (r == -1 && errno == EINTR);

    if (r == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status))
        printf("[PARENT] Child exited with status: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("[PARENT] Child killed by signal %d\n", WTERMSIG(status));
    else
        printf("[PARENT] Child ended abnormally\n");

    printf("[PARENT] Child has finished. This is the last line printed by parent.\n");
    return EXIT_SUCCESS;
#endif
}
