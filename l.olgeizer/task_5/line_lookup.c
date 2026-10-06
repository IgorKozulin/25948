#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

/* Структура для хранения смещения и длины строки */
typedef struct {
    off_t offset; // Смещение от начала файла в байтах
    int length;   // Длина строки (включая символ '\n')
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    int capacity = 16;
    int num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        close(fd);
        return EXIT_FAILURE;
    }

    off_t line_start = 0;
    char ch;

    /* 1. Построение таблицы смещений */
    while (read(fd, &ch, 1) > 0) {
        if (ch == '\n') {
            off_t next_line_start = lseek(fd, 0L, 1); // 1 == SEEK_CUR: текущая позиция сразу после '\n'

            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (temp == NULL) {
                    perror("realloc");
                    free(table);
                    close(fd);
                    return EXIT_FAILURE;
                }
                table = temp;
            }

            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(next_line_start - line_start);
            num_lines++;

            line_start = next_line_start;
        }
    }

    /* Обработка последней строки, если файл заканчивается без перевода строки '\n' */
    off_t file_end = lseek(fd, 0L, 1);
    if (file_end > line_start) {
        if (num_lines >= capacity) {
            capacity += 1;
            LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            if (temp != NULL) {
                table = temp;
            }
        }
        if (num_lines < capacity) {
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(file_end - line_start);
            num_lines++;
        }
    }

    /* 2. Отладочный вывод таблицы смещений */
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, (long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    /* 3. Интерактивный запрос номера строки */
    int req_line;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &req_line) != 1) {
            break;
        }

        if (req_line == 0) {
            break;
        }

        if (req_line < 1 || req_line > num_lines) {
            fprintf(stderr, "Недопустимый номер строки. Диапазон: 1..%d\n", num_lines);
            continue;
        }

        LineInfo info = table[req_line - 1];

        /* Позиционирование на начало строки */
        if (lseek(fd, info.offset, SEEK_SET) == (off_t)-1) {
            perror("lseek error");
            continue;
        }

        /* Выделение буфера и чтение ровно length байт */
        char *buf = (char *)malloc(info.length + 1);
        if (buf == NULL) {
            perror("malloc error");
            continue;
        }

        ssize_t bytes_read = read(fd, buf, info.length);
        if (bytes_read == -1) {
            perror("read error");
            free(buf);
            continue;
        }

        buf[bytes_read] = '\0';
        printf("%s", buf);
        if (bytes_read > 0 && buf[bytes_read - 1] != '\n') {
            printf("\n");
        }

        free(buf);
    }

    /* 4. Очистка ресурсов */
    free(table);
    close(fd);

    return EXIT_SUCCESS;
}
