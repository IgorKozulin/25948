#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Глобальные переменные для обработчика сигнала
char *global_map;
size_t global_size;

typedef struct {
    long offset;
    int length;
} LineInfo;

// Обработчик сигнала таймаута
void alarm_handler(int sig) {
    printf("\n[TIMEOUT] Time is up! Printing full file content...\n");
    // Выводим всё содержимое напрямую из отображенной памяти
    write(STDOUT_FILENO, global_map, global_size);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Открытие файла и получение размера
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    global_size = st.st_size;

    // Отображение файла в память
    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    // Дескриптор больше не нужен, отображение сохраняется
    close(fd);

    // Построение таблицы строк
    int num_lines = 0;
    int capacity = 10;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    long line_start = 0;

    for (size_t i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start;
            num_lines++;
            line_start = i + 1;
        }
    }

    // Обработка последней строки, если она не заканчивается на '\n'
    if (line_start < global_size) {
        if (num_lines >= capacity) {
            capacity *= 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = global_size - line_start;
        num_lines++;
    }

    // Отладочный вывод
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // Регистрация обработчика сигнала
    signal(SIGALRM, alarm_handler);

    // Интерактивный цикл
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5); // Запуск таймера

        int line_num;
        if (scanf("%d", &line_num) != 1) {
            // Очистка буфера ввода при вводе не-числа
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            alarm(0); // Сброс таймера
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        alarm(0); // Пользователь успел, отменяем таймер

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Line number out of range (1..%d)\n", num_lines);
            continue;
        }

        // Вывод строки напрямую из памяти через fwrite
        long offset = table[line_num - 1].offset;
        int length = table[line_num - 1].length;
        
        fwrite(global_map + offset, 1, length, stdout);
        printf("\n"); // Добавляем перенос строки для красоты вывода
    }

    // Очистка ресурсов
    free(table);
    munmap(global_map, global_size);

    return 0;
}
