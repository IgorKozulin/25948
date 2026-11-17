#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Структура для хранения информации о строке
typedef struct {
    long offset;
    int length;
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }

    int capacity = 10;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Построение таблицы смещений и длин
    long line_start = 0;
    int current_length = 0;
    char c;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &c, 1)) > 0) {
        current_length++;
        
        if (c == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_length;
            num_lines++;

            line_start = lseek(fd, 0L, SEEK_CUR);
            current_length = 0;
        }
    }

    if (current_length > 0) {
        if (num_lines >= capacity) {
            capacity++;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_length;
        num_lines++;
    }

    // --- НОВОЕ: Подсчет общего размера и вывод в битах ---
    long total_bytes = 0;
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        long bits = table[i].length * 8; // Перевод байтов в биты
        total_bytes += table[i].length;
        printf("Line %d: Offset = %ld, Length = %d bytes (%ld bits)\n", 
               i + 1, table[i].offset, table[i].length, bits);
    }
    printf("-------------------------\n");

    // Интерактивный запрос номера строки
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            while (getchar() != '\n'); 
            continue;
        }

        // --- НОВОЕ: Вывод общего размера файла при выходе ---
        if (line_num == 0) {
            long total_bits = total_bytes * 8;
            printf("\nЗавершение работы.\n");
            printf("Общий размер файла: %ld байт (%ld бит).\n", total_bytes, total_bits);
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Invalid line number. File has %d lines.\n", num_lines);
            continue;
        }

        int idx = line_num - 1;
        long offset = table[idx].offset;
        int length = table[idx].length;

        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования (lseek)");
            continue;
        }

        char *buffer = malloc(length + 1);
        if (buffer == NULL) continue;

        if (read(fd, buffer, length) != length) {
            perror("Ошибка при чтении строки");
            free(buffer);
            continue;
        }

        buffer[length] = '\0';
        printf("%s", buffer);

        if (buffer[length - 1] != '\n') {
            printf("\n");
        }

        // --- НОВОЕ: Информационный вывод о размере конкретной строки ---
        printf("[Размер запрошенной строки: %d бит]\n", length * 8);

        free(buffer);
    }

    free(table);
    close(fd);

    return 0;
}
