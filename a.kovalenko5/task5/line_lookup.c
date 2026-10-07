#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    off_t offset;      // начало строки в файле
    size_t length;     // длина строки без '\n'
} LineInfo;

int main(int argc, char *argv[])
{
    // Проверяем, передано ли имя файла
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Открываем файл только для чтения
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    LineInfo *table = NULL;

    size_t num_lines = 0;
    size_t capacity = 0;

    off_t line_start = 0;
    size_t line_length = 0;

    char ch;
    ssize_t bytes_read;

    // Читаем файл по одному символу
    while ((bytes_read = read(fd, &ch, 1)) > 0) {

        if (ch == '\n') {

            // Если места в таблице нет — увеличиваем её
            if (num_lines == capacity) {
                if (capacity == 0)
                    capacity = 10;
                else
                    capacity *= 2;

                LineInfo *temp =
                    realloc(table, capacity * sizeof(LineInfo));

                if (temp == NULL) {
                    printf("Memory allocation error\n");

                    free(table);
                    close(fd);

                    return 1;
                }

                table = temp;
            }

            // Сохраняем информацию о строке
            table[num_lines].offset = line_start;
            table[num_lines].length = line_length;

            num_lines++;

            // Узнаём текущую позицию в файле
            // После чтения '\n' это уже начало следующей строки
            line_start = lseek(fd, 0L, SEEK_CUR);

            if (line_start == (off_t)-1) {
                perror("lseek");

                free(table);
                close(fd);

                return 1;
            }

            line_length = 0;
        }
        else {
            line_length++;
        }
    }

    if (bytes_read == -1) {
        perror("read");

        free(table);
        close(fd);

        return 1;
    }

    /*
        Если файл закончился без '\n',
        нужно отдельно добавить последнюю строку.
    */
    if (line_length > 0) {

        if (num_lines == capacity) {
            if (capacity == 0)
                capacity = 10;
            else
                capacity *= 2;

            LineInfo *temp =
                realloc(table, capacity * sizeof(LineInfo));

            if (temp == NULL) {
                printf("Memory allocation error\n");

                free(table);
                close(fd);

                return 1;
            }

            table = temp;
        }

        table[num_lines].offset = line_start;
        table[num_lines].length = line_length;

        num_lines++;
    }

    // --------------------------
    // Вывод таблицы для отладки
    // --------------------------

    printf("\n--- Line Table ---\n");

    for (size_t i = 0; i < num_lines; i++) {
        printf(
            "Line %zu: Offset = %lld, Length = %zu\n",
            i + 1,
            (long long)table[i].offset,
            table[i].length
        );
    }

    printf("------------------\n");

    // --------------------------
    // Запрос номера строки
    // --------------------------

    int line_number;

    while (1) {

        printf("\nEnter line number (0 to quit): ");

        if (scanf("%d", &line_number) != 1) {
            printf("Invalid input\n");
            break;
        }

        // 0 завершает программу
        if (line_number == 0) {
            break;
        }

        // Проверяем существование такой строки
        if (line_number < 1 ||
            line_number > (int)num_lines) {

            printf("No such line\n");
            continue;
        }

        // Индекс массива начинается с 0
        size_t index = line_number - 1;

        off_t offset = table[index].offset;
        size_t length = table[index].length;

        // Переходим сразу к началу нужной строки
        if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            break;
        }

        // +1 нужен для '\0'
        char *buffer = malloc(length + 1);

        if (buffer == NULL) {
            printf("Memory allocation error\n");
            break;
        }

        // Читаем ровно длину строки
        ssize_t result = read(fd, buffer, length);

        if (result == -1) {
            perror("read");

            free(buffer);
            break;
        }

        // read() сам '\0' не добавляет
        buffer[result] = '\0';

        printf("%s\n", buffer);

        free(buffer);
    }

    // Освобождаем ресурсы
    free(table);
    close(fd);

    return 0;
}
