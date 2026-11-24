/* Задача 25: родитель пишет текст в pipe, потомок печатает его в верхнем регистре */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHUNK        1024
#define LINE_CAPACITY 4096

static int write_all(int fd, const char *buf, size_t len)
{
    size_t done = 0;

    while (done < len) {
        ssize_t n = write(fd, buf + done, len - done);
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        done += (size_t)n;
    }
    return 0;
}

/* Работает в потомке: читает канал, переводит в верхний регистр, печатает по строкам */
static void child_worker(int rfd)
{
    char chunk[CHUNK];
    char line[LINE_CAPACITY];
    size_t linelen = 0;
    ssize_t n;

    printf("[CHILD] Запущен. Ожидаю данные от родителя...\n");
    fflush(stdout);

    for (;;) {
        n = read(rfd, chunk, sizeof chunk);
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("[CHILD] read");
            _exit(EXIT_FAILURE);
        }
        if (n == 0)
            break;                       /* EOF: все концы записи закрыты */

        for (ssize_t i = 0; i < n; i++) {
            int up = toupper((unsigned char)chunk[i]);   /* приведение обязательно */

            if (up == '\n') {
                line[linelen] = '\0';
                printf("[CHILD] Получено и преобразовано: %s\n", line);
                fflush(stdout);
                linelen = 0;
            } else if (linelen + 1 < sizeof line) {
                line[linelen++] = (char)up;
            } else {                     /* строка длиннее буфера — сбрасываем куском */
                line[linelen] = '\0';
                printf("[CHILD] Получено и преобразовано: %s\n", line);
                fflush(stdout);
                linelen = 0;
                line[linelen++] = (char)up;
            }
        }
    }

    if (linelen > 0) {                   /* последняя строка без '\n' */
        line[linelen] = '\0';
        printf("[CHILD] Получено и преобразовано: %s\n", line);
    }
    fflush(stdout);
    close(rfd);
    _exit(EXIT_SUCCESS);                 /* не exit(): не сбрасываем чужой буфер */
}

int main(void)
{
    int pipefd[2];
    pid_t pid;
    char *line = NULL;
    size_t cap = 0;
    int rc = EXIT_SUCCESS;
    int interactive;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    fflush(NULL);                        /* до fork(): буферы не должны раздваиваться */

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {                      /* ---------- потомок ---------- */
        close(pipefd[1]);                /* ему нужен только конец чтения */
        child_worker(pipefd[0]);
        _exit(EXIT_FAILURE);             /* недостижимо */
    }

    /* ---------- родитель ---------- */
    close(pipefd[0]);                    /* ему нужен только конец записи */

    signal(SIGPIPE, SIG_IGN);            /* смерть потомка не должна убивать родителя */

    interactive = isatty(STDIN_FILENO);

    printf("[PARENT] Связь через канал установлена!\n");
    printf("[PARENT] Введите текст (пустая строка или Ctrl+D — выход):\n");
    fflush(stdout);

    for (;;) {
        ssize_t got;

        if (interactive) {
            printf("> ");
            fflush(stdout);
        }

        got = getline(&line, &cap, stdin);
        if (got == -1) {
            if (ferror(stdin))
                perror("[PARENT] getline");
            else
                printf("\n[PARENT] Конец ввода (EOF).\n");
            break;
        }

        if (got > 0 && line[got - 1] == '\n')
            line[--got] = '\0';

        if (got == 0) {
            printf("[PARENT] Пустая строка — завершаю ввод.\n");
            break;
        }

        if (write_all(pipefd[1], line, (size_t)got) == -1 ||
            write_all(pipefd[1], "\n", 1) == -1) {
            perror("[PARENT] write");
            rc = EXIT_FAILURE;
            break;
        }
    }

    close(pipefd[1]);                    /* вот это и даёт потомку EOF */

    {
        int status = 0;
        while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
            ;
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            rc = EXIT_FAILURE;
    }

    printf("[PARENT] Потомок завершился. Родитель тоже завершает работу.\n");
    free(line);
    return rc;
}
