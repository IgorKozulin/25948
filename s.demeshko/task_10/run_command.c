/* Задача 10: запуск команды из argv[1] и печать её кода завершения */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(NULL);   /* до fork(): иначе при перенаправлении буфер задвоится */

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {                        /* ---------- ребёнок ---------- */
        /* argv[0] — это наша программа-обёртка, поэтому начинаем с argv[1].
           Массив argv всегда завершён NULL — его и передаём как есть. */
        execvp(argv[1], &argv[1]);

        /* Сюда попадаем, только если exec не удался */
        fprintf(stderr, "[CHILD] execvp failed: %s\n", strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
    }

    /* ---------- родитель ---------- */
    int status = 0;
    pid_t r;
    do {
        r = waitpid(pid, &status, 0);
    } while (r == -1 && errno == EINTR);

    if (r == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("[PARENT] Command exited normally with code: %d\n", code);
        return code;                     /* обёртка пробрасывает код дальше */
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
#ifdef WCOREDUMP
        const char *core = WCOREDUMP(status) ? " [core dumped]" : "";
#else
        const char *core = "";
#endif
        printf("[PARENT] Command terminated by signal: %d (%s)%s\n",
               sig, strsignal(sig), core);
        return 128 + sig;                /* как делает shell */
    }

    printf("[PARENT] Command ended abnormally (status=0x%x)\n", status);
    return EXIT_FAILURE;
}
