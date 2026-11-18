#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(void) {
    int chan[2];
    pid_t child;

    if (pipe(chan) == -1) {
        perror("pipe");
        return 1;
    }

    child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }

    if (child == 0) {
        close(chan[1]);

        printf("[CHILD] Запущен. Ожидаю данные от родителя...\n");
        fflush(stdout);

        char chunk[256];
        ssize_t got;

        while ((got = read(chan[0], chunk, sizeof(chunk) - 1)) > 0) {
            chunk[got] = '\0';
            for (ssize_t k = 0; k < got; k++) {
                chunk[k] = (char)toupper((unsigned char)chunk[k]);
            }
            printf("[CHILD] Получено и преобразовано: %s", chunk);
            fflush(stdout);
        }

        close(chan[0]);
        printf("[CHILD] Конец данных. Завершаюсь.\n");
        exit(EXIT_SUCCESS);
    }

    close(chan[0]);

    printf("[PARENT] Связь через канал установлена!\n");
    printf("[PARENT] Введите текст (пустая строка или Ctrl+D — выход):\n");
    fflush(stdout);

    char line[256];
    for (;;) {
        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n[PARENT] Получен EOF (Ctrl+D).\n");
            break;
        }

        if (line[0] == '\n' || line[0] == '\0') {
            printf("[PARENT] Пустая строка — завершаю ввод.\n");
            break;
        }

        write(chan[1], line, strlen(line));
    }

    close(chan[1]);

    wait(NULL);
    printf("[PARENT] Потомок завершился. Родитель тоже завершает работу.\n");

    return 0;
}

