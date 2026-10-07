#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

// 1. Структура для хранения информации о строке
typedef struct {
    long offset;
    int length;
} LineInfo;

// Глобальная переменная для файлового дескриптора, 
// чтобы к ней был доступ из обработчика сигнала.
int global_fd = -1;

// 3. Обработчик сигнала SIGALRM
void alarm_handler(int sig) {
    // Используем write() вместо printf(), так как printf() не является 
    // асинхронно-безопасной функцией (async-signal-safe).
    char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (global_fd != -1) {
        // Перемещаемся в начало файла
        lseek(global_fd, 0, SEEK_SET);

        // Читаем и выводим весь файл блоками
        char buf[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(global_fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
        }
    }

    // Завершаем процесс без вызова стандартных функций очистки (например, fflush)
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Открытие файла
    global_fd = open(argv[1], O_RDONLY);
    if (global_fd == -1) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }

    // Инициализация динамического массива для таблицы
    int capacity = 10;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти");
        close(global_fd);
        exit(EXIT_FAILURE);
    }

    // 2. Построение таблицы смещений и длин
    long line_start = 0;
    int current_length = 0;
    char c;
    ssize_t bytes_read;

    while ((bytes_read = read(global_fd, &c, 1)) > 0) {
        current_length++;
        
        if (c == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (table == NULL) {
                    perror("Ошибка перераспределения памяти");
                    close(global_fd);
                    exit(EXIT_FAILURE);
                }
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_length;
            num_lines++;
            line_start = lseek(global_fd, 0L, SEEK_CUR);
            current_length = 0;
        }
    }

    // Обработка последней строки без '\n'
    if (current_length > 0) {
        if (num_lines >= capacity) {
            capacity++;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_length;
        num_lines++;
    }

    // Вывод отладочной таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // Устанавливаем обработчик сигнала SIGALRM
    signal(SIGALRM, alarm_handler);

    // 4. Интерактивный цикл с таймером
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout); // Обязательно сбрасываем буфер, чтобы текст появился до блокировки в scanf!

        // Запускаем таймер на 5 секунд
        alarm(5);

        // Ожидаем ввод от пользователя
        if (scanf("%d", &line_num) != 1) {
            // Если введено не число
            alarm(0); // Останавливаем таймер
            while (getchar() != '\n'); // Очищаем буфер
            continue;
        }

        // Пользователь успел ввести данные — выключаем таймер!
        alarm(0);

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Invalid line number. File has %d lines.\n", num_lines);
            continue;
        }

        // Чтение и вывод конкретной строки
        int idx = line_num - 1;
        long offset = table[idx].offset;
        int length = table[idx].length;

        if (lseek(global_fd, offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования (lseek)");
            continue;
        }

        char *buffer = malloc(length + 1);
        if (buffer == NULL) {
            perror("Ошибка выделения памяти для буфера");
            continue;
        }

        if (read(global_fd, buffer, length) != length) {
            perror("Ошибка при чтении строки");
            free(buffer);
            continue;
        }

        buffer[length] = '\0';
        printf("%s", buffer);

        if (buffer[length - 1] != '\n') {
            printf("\n");
        }

        free(buffer);
    }

    // 5. Завершение работы
    free(table);
    close(global_fd);

    return 0;
}
