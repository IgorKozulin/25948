#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Структура для хранения информации о строке
typedef struct {
    long offset;  // Смещение от начала файла
    int length;   // Длина строки
} LineInfo;

int main(int argc, char *argv[]) {
    // Открытие файла
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Построение таблицы (первый проход)
    int num_lines = 0;
    int capacity = 10;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));

    char ch;
    long current_pos = 0;
    long line_start = 0;

    // Читаем файл побайтно
    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            // Расширяем массив, если не хватает места
            if (num_lines >= capacity) {
                capacity *= 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            // Записываем данные в таблицу
            table[num_lines].offset = line_start;
            table[num_lines].length = current_pos - line_start;
            num_lines++;
            line_start = current_pos + 1; // Начало следующей строки
        }
        current_pos++;
    }

    // Добавляем последнюю строку, если она не заканчивается на '\n'
    if (current_pos > line_start) {
        if (num_lines >= capacity) {
            capacity *= 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_pos - line_start;
        num_lines++;
    }

    // Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // Интерактивный запрос
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            break; // Защита от бесконечного цикла при вводе букв
        }

        if (line_num == 0) {
            break; // Завершение программы
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Line number out of range (1..%d)\n", num_lines);
            continue;
        }

        // Чтение и вывод конкретной строки
        long offset = table[line_num - 1].offset;
        int length = table[line_num - 1].length;

        // Позиционируемся на начало нужной строки
        lseek(fd, offset, SEEK_SET);

        // Выделяем буфер и читаем ровно length байт
        char *buffer = (char *)malloc(length + 1);
        read(fd, buffer, length);
        
        buffer[length] = '\0'; // Добавляем нулевой байт вручную!

        printf("%s\n", buffer); // Выводим (символ \n уже есть внутри buffer)

        free(buffer); // Освобождаем буфер
    }

    // Очистка ресурсов
    free(table);
    close(fd);

    return 0;
}
