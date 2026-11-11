#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];  // pipefd[0] — читать, pipefd[1] — писать
    pid_t pid;

    // 1. Создаем программный канал
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    // 2. Создаем дочерний процесс
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    // ==========================================
    // ЛОГИКА ДОЧЕРНЕГО ПРОЦЕССА (ЧИТАТЕЛЯ)
    // ==========================================
    if (pid == 0) {
        // Закрываем конец записи — он нам не нужен
        close(pipefd[1]);

        printf("[CHILD] Запущен. Ожидаю данные от родителя...\n");
        fflush(stdout);

        char buf[256];
        ssize_t n;

        // Читаем из канала, пока не получим EOF (0)
        while ((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            // Переводим каждый символ в верхний регистр
            for (int i = 0; i < n; i++) {
                // Приведение к unsigned char обязательно для безопасности
                buf[i] = toupper((unsigned char)buf[i]);
            }
            printf("[CHILD] Получено и преобразовано: %s", buf);
            fflush(stdout);
        }

        close(pipefd[0]);
        printf("[CHILD] Конец данных. Завершаюсь.\n");
        exit(EXIT_SUCCESS);
    }

    // ==========================================
    // ЛОГИКА РОДИТЕЛЬСКОГО ПРОЦЕССА (ПИСАТЕЛЯ)
    // ==========================================
    else {
        // Закрываем конец чтения — он нам не нужен
        close(pipefd[0]);

        printf("[PARENT] Связь через канал установлена!\n");
        printf("[PARENT] Введите текст (пустая строка или Ctrl+D — выход):\n");
        fflush(stdout);

        char input[256];
        while (1) {
            printf("> ");
            fflush(stdout);

            // Читаем строку с клавиатуры
            if (fgets(input, sizeof(input), stdin) == NULL) {
                // Пользователь нажал Ctrl+D
                printf("\n[PARENT] Получен EOF (Ctrl+D).\n");
                break;
            }

            // Если пустая строка — выходим
            if (input[0] == '\n' || input[0] == '\0') {
                printf("[PARENT] Пустая строка — завершаю ввод.\n");
                break;
            }

            // Отправляем введенную строку в канал
            write(pipefd[1], input, strlen(input));
        }

        // Закрываем конец записи. Это отправит потомку EOF, 
        // и его read вернет 0.
        close(pipefd[1]);

        // Ждем завершения потомка
        wait(NULL);
        printf("[PARENT] Потомок завершился. Родитель тоже завершает работу.\n");
    }

    return 0;
}
