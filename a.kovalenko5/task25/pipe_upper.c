#include <sys/types.h> // Тип pid_t
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2];

    // Создаем программный канал
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    // Создаем дочерний процесс
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс читает из канала
        close(pipefd[1]);

        char buffer[256];
        ssize_t n;

        // Счетчики букв, цифр и всех символов
        int letters = 0;
        int digits = 0;
        int total = 0;

        // Читаем данные из канала
        while (1) {
            n = read(pipefd[0], buffer, sizeof(buffer));

            if (n == 0) {
                break;
            }

            if (n == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("read");
                close(pipefd[0]);
                _exit(1);
            }

            // Подсчитываем символы и меняем регистр
            for (ssize_t i = 0; i < n; i++) {
                if (isalpha((unsigned char)buffer[i])) {
                    letters++;
                }

                if (isdigit((unsigned char)buffer[i])) {
                    digits++;
                }

                if (buffer[i] != '\n') {
                    total++;
                }

                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            // Выводим преобразованный текст
            ssize_t sent = 0;

            while (sent < n) {
                ssize_t k = write(STDOUT_FILENO,
                                  buffer + sent, n - sent);

                if (k == -1 && errno == EINTR) {
                    continue;
                }

                if (k <= 0) {
                    perror("write");
                    close(pipefd[0]);
                    _exit(1);
                }

                sent += k;
            }
        }

        // Выводим итоговую статистику
        printf("\nСтатистика:\n");
        printf("Букв: %d\n", letters);
        printf("Цифр: %d\n", digits);
        printf("Всего символов: %d\n", total);
        fflush(stdout);

        // Закрываем канал и завершаем потомка
        close(pipefd[0]);
        _exit(0);
    }

    // Родительский процесс записывает в канал
    close(pipefd[0]);

    char buffer[256];

    printf("Введите текст (пустая строка - выход):\n");
    fflush(stdout);

    // Читаем текст с клавиатуры
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (buffer[0] == '\n') {
            break;
        }

        size_t len = strlen(buffer);
        size_t sent = 0;

        // Отправляем текст дочернему процессу
        while (sent < len) {
            ssize_t k = write(pipefd[1],
                              buffer + sent, len - sent);

            if (k == -1 && errno == EINTR) {
                continue;
            }

            if (k <= 0) {
                perror("write");
                close(pipefd[1]);
                waitpid(pid, NULL, 0);
                return 1;
            }

            sent += k;
        }
    }

    // Закрываем канал и ждем потомка
    close(pipefd[1]);
    waitpid(pid, NULL, 0);

    printf("Программа завершена.\n");

    return 0;
}

