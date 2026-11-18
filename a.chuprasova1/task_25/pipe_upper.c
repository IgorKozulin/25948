
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2]; // создание канала 0 -чтенеи 1- запись

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork(); //новый процесс копия

    if (pid == -1) { // ошибка
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }

    if (pid == 0) { // дочерний
        close(pipefd[1]); // закрываем запись
        printf("[CHILD] Дочерний процесс, PID = %ld\n", (long)getpid());
        fflush(stdout);

        char buffer[256];
        ssize_t n;

        //счетчики статистики
        int total_letters = 0;
        int changed_letters = 0;
        int upper_letters = 0;
        int digits = 0;


        while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) { //чтение
            printf("[CHILD] Получено %zd байт из канала\n", n);
            fflush(stdout);

            for (ssize_t i = 0; i < n; i++) {

                //подсчет символов до изменения регистра
                unsigned char ch = (unsigned char)buffer[i];

                if (ch >= 'a' && ch <= 'z') {
                    total_letters++;
                    changed_letters++;
                }
                else if (ch >= 'A' && ch <= 'Z') {
                    total_letters++;
                    upper_letters++;
                }
                else if (ch >= '0' && ch <= '9') {
                    digits++;
                }

                buffer[i] = toupper((unsigned char)buffer[i]);
            }

            printf("[CHILD] Результат: ");
            fflush(stdout);

            if (write(STDOUT_FILENO, buffer, n) == -1) { //вывод
                perror("write");
                close(pipefd[0]);
                return 1;
            }
        }

        if (n == -1) {
            perror("read");
            close(pipefd[0]);
            return 1;
        }

        printf("\n[CHILD] Статистика:\n");
        printf("Всего букв: %d\n", total_letters);
        printf("Изменено на верхний регистр: %d\n", changed_letters);
        printf("Уже были в верхнем регистре: %d\n", upper_letters);
        printf("Цифр: %d\n", digits);

        close(pipefd[0]);
        return 0;
    }

    close(pipefd[0]); //род не читает канал
    printf("[PARENT] Родительский процесс, PID = %ld\n", (long)getpid());
    printf("[PARENT] Создан потомок, PID = %ld\n", (long)pid);

    char buffer[256];

    printf("Введите текст (пустая строка - выход):\n");
    fflush(stdout);

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) { //чтение с клавиатуры
        if (strcmp(buffer, "\n") == 0) { //entr
            break;
        }

        size_t len = strlen(buffer);
        size_t sent = 0; //кол отправл байт в канал

        printf("[PARENT] Отправляю в канал: %s", buffer);
        fflush(stdout);

        while (sent < len) {
            ssize_t n = write(pipefd[1], buffer + sent, len - sent); //запись в канал 

            if (n <= 0) {
                perror("write");
                close(pipefd[1]);
                wait(NULL); //ожидание дочернего
                return 1;
            }

            sent += (size_t)n;
        }
    }

    close(pipefd[1]);

    printf("[PARENT] Закрытие канала\n");
    fflush(stdout);

    wait(NULL);

    printf("Программа завершена.\n");
    return 0;
}

