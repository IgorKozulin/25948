
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(void)
{
    int pipefd[2];
    pid_t pid;
    char buffer[256];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]);

        ssize_t bytes_read;

        while ((bytes_read = read(pipefd[0], buffer,
                                  sizeof(buffer))) > 0) {
            for (ssize_t i = 0; i < bytes_read; i++) {
                buffer[i] = (char)toupper((unsigned char)buffer[i]);
            }

            const char *message =
                "[CHILD] Получено и преобразовано: ";

            write(STDOUT_FILENO, message, strlen(message));
            write(STDOUT_FILENO, buffer, bytes_read);

            if (buffer[bytes_read - 1] != '\n') {
                write(STDOUT_FILENO, "\n", 1);
            }
        }

        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    } else {
        close(pipefd[0]);

        printf("[PARENT] Связь через канал установлена!\n");
        printf("[PARENT] Введите текст (пустая строка или Ctrl+D — выход):\n");
        fflush(stdout);

        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (strcmp(buffer, "\n") == 0) {
                printf("[PARENT] Пустая строка — завершаю ввод.\n");
                break;
            }

            size_t length = strlen(buffer);
            write(pipefd[1], buffer, length);
        }

        close(pipefd[1]);
        wait(NULL);

        printf("[PARENT] Потомок завершился. Родитель тоже завершает работу.\n");
    }

    return 0;
}

