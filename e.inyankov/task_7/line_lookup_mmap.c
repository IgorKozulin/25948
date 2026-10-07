#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

// 1. Структура для хранения информации о строке
typedef struct {
    long offset;
    int length;
} LineInfo;

// Глобальные переменные для доступа к данным из обработчика сигнала
char *global_map = NULL;
size_t global_size = 0;

// 4. Обработчик сигнала SIGALRM
void alarm_handler(int sig) {
    // Используем асинхронно-безопасный вызов write
    char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (global_map != NULL && global_size > 0) {
        // Выводим все содержимое файла, отображенное в память
        write(STDOUT_FILENO, global_map, global_size);
    }

    // Завершаем процесс
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 1. Открытие и получение размера
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Ошибка fstat");
        close(fd);
        exit(EXIT_FAILURE);
    }

    global_size = st.st_size;

    if (global_size == 0) {
        printf("File is empty.\n");
        close(fd);
        exit(EXIT_SUCCESS);
    }

    // 2. Отображение в память
    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("Ошибка mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Дескриптор больше не нужен, память уже отображена
    close(fd);

    // Инициализация таблицы строк
    int capacity = 10;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("Ошибка выделения памяти");
        munmap(global_map, global_size);
        exit(EXIT_FAILURE);
    }

    // 3. Построение таблицы строк через прямой доступ к памяти
    long line_start = 0;
    int current_length = 0;

    for (size_t i = 0; i < global_size; i++) {
        current_length++;

        if (global_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (table == NULL) {
                    perror("Ошибка перераспределения памяти");
                    munmap(global_map, global_size);
                    exit(EXIT_FAILURE);
                }
            }
            
            table[num_lines].offset = line_start;
            table[num_lines].length = current_length;
            num_lines++;
            
            line_start = i + 1;
            current_length = 0;
        }
    }

    // Обработка последней строки, если она не завершается '\n'
    if (current_length > 0) {
        if (num_lines >= capacity) {
            capacity++;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_length;
        num_lines++;
    }

    // Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // Установка обработчика сигнала SIGALRM
    signal(SIGALRM, alarm_handler);

    // 5. Интерактивный цикл
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout); // Обязательный сброс буфера перед ожиданием ввода

        alarm(5); // Запускаем таймер на 5 секунд

        if (scanf("%d", &line_num) != 1) {
            alarm(0); // Отключаем таймер
            while (getchar() != '\n'); // Очищаем некорректный ввод
            continue;
        }

        alarm(0); // Ввод успешен, отключаем таймер

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Invalid line number. File has %d lines.\n", num_lines);
            continue;
        }

        // Вывод нужной строки прямо из памяти
        int idx = line_num - 1;
        long offset = table[idx].offset;
        int length = table[idx].length;

        // Используем fwrite, так как данные в mmap не имеют '\0' на конце каждой строки
        fwrite(global_map + offset, 1, length, stdout);

        // Если последняя строка файла не содержит '\n', добавляем перенос для корректного отображения
        if (global_map[offset + length - 1] != '\n') {
            printf("\n");
        }
    }

    // 6. Очистка ресурсов
    free(table);
    munmap(global_map, global_size);

    return 0;
}
