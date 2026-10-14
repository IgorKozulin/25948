#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

// информация о одной строке
typedef struct {
    long offset;    // смещение от начала файла
    int length;     // длина без \n
} LineInfo;

#define INITIAL_CAPACITY 16

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Строим таблица строк
    LineInfo *table = malloc(INITIAL_CAPACITY * sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }
    int capacity = INITIAL_CAPACITY;
    int num_lines = 0;

    long line_start = 0;        //смещение начала текущей строки
    long current_pos = 0;       //позиция в файле
    char ch;

    while (1) {
        ssize_t n = read(fd, &ch, 1);
        if (n == -1) {
            perror("read");
            free(table);
            close(fd);
            return 1;
        }
        if (n == 0) {
            // конец файла, если осталась непустая строка без \n добавляем её 
            if (current_pos > line_start) {
                if (num_lines >= capacity) {
                    capacity *= 2;
                    table = realloc(table, capacity * sizeof(LineInfo));
                    if (table == NULL) {
                        perror("realloc");
                        close(fd);
                        return 1;
                    }
                }
                table[num_lines].offset = line_start;
                table[num_lines].length = (int)(current_pos - line_start);
                num_lines++;
            }
            break;
        }

        if (ch == '\n') {
            // записываем строку в таблицу 
            if (num_lines >= capacity) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (table == NULL) {
                    perror("realloc");
                    close(fd);
                    return 1;
                }
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(current_pos - line_start);
            num_lines++;

            // следующая строка начинается после \n
            line_start = current_pos + 1;
        }

        current_pos++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // интерактивный запрос
    while (1) {
        int line_num;
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            //некорректный ввод
            while (getchar() != '\n');
            continue;
        }

        if (line_num == 0) {
            break;
        }
        if (line_num < 1 || line_num > num_lines) {
            printf("Line %d out of range (1..%d)\n", line_num, num_lines);
            continue;
        }

        //читаем нужную строку
        int idx = line_num - 1;
        if (lseek(fd, table[idx].offset, SEEK_SET) == -1) {
            perror("lseek");
            break;
        }

        int len = table[idx].length;
        char *buf = malloc(len + 1);
        if (buf == NULL) {
            perror("malloc");
            break;
        }

        ssize_t n = read(fd, buf, len);
        if (n == -1) {
            perror("read");
            free(buf);
            break;
        }
        buf[n] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    free(table);
    close(fd);
    return 0;
}
