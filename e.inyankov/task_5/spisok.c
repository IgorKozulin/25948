#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// 1. Структура для хранения информации о строке
typedef struct {
    long offset;
    int length;
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 2. Открытие файла с использованием системного вызова open(2)
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }

    // Инициализация динамического массива для таблицы строк
    int capacity = 10;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 3. Построение таблицы смещений и длин
    long line_start = 0;
    int current_length = 0;
    char c;
    ssize_t bytes_read;

    // Читаем файл по одному символу
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        current_length++;
        
        if (c == '\n') {
            // Если массив заполнен, увеличиваем его
            if (num_lines >= capacity) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (table == NULL) {
                    perror("Ошибка перераспределения памяти");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
            }

            // Записываем данные в таблицу
            table[num_lines].offset = line_start;
            table[num_lines].length = current_length;
            num_lines++;

            // Получаем текущую позицию с помощью lseek, как указано в подсказке.
            // SEEK_CUR (1) означает смещение относительно текущей позиции.
            line_start = lseek(fd, 0L, SEEK_CUR);
            current_length = 0;
        }
    }

    if (bytes_read == -1) {
        perror("Ошибка при чтении файла");
        free(table);
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Обработка последней строки, если она не заканчивается символом '\n'
    if (current_length > 0) {
        if (num_lines >= capacity) {
            capacity++;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_length;
        num_lines++;
    }

    // 4. Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // 5. Интерактивный запрос номера строки
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            // Очистка буфера ввода при вводе нечисловых данных
            while (getchar() != '\n'); 
            continue;
        }

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Invalid line number. File has %d lines.\n", num_lines);
            continue;
        }

        // 6. Чтение и вывод конкретной строки
        int idx = line_num - 1;
        long offset = table[idx].offset;
        int length = table[idx].length;

        // Позиционируемся на начало нужной строки
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Ошибка позиционирования (lseek)");
            continue;
        }

        // Выделяем память под строку + 1 байт для '\0'
        char *buffer = malloc(length + 1);
        if (buffer == NULL) {
            perror("Ошибка выделения памяти для буфера");
            continue;
        }

        // Читаем точную длину строки
        if (read(fd, buffer, length) != length) {
            perror("Ошибка при чтении строки");
            free(buffer);
            continue;
        }

        // Завершаем строку нулевым символом и выводим
        buffer[length] = '\0';
        printf("%s", buffer);

        // Если последняя строка файла не содержит '\n', добавляем перенос для красоты вывода
        if (buffer[length - 1] != '\n') {
            printf("\n");
        }

        free(buffer);
    }

    // 7. Очистка ресурсов
    free(table);
    close(fd);

    return 0;
}
