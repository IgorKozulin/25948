#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

typedef struct {
    long offset;
    int  length;
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int capacity = 16;
    int num_lines = 0;
    LineInfo *table = malloc(sizeof(LineInfo) * capacity);
    if (table == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    long line_start = 0;
    long current_pos = 0;
    char c;
    ssize_t n;

    while ((n = read(fd, &c, 1)) == 1) {
        if (c == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *tmp = realloc(table, sizeof(LineInfo) * capacity);
                if (tmp == NULL) {
                    perror("realloc");
                    free(table);
                    close(fd);
                    return 1;
                }
                table = tmp;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(current_pos - line_start);
            num_lines++;
            line_start = current_pos + 1;
        }
        current_pos++;
    }

    if (n == -1) {
        perror("read");
        free(table);
        close(fd);
        return 1;
    }

    if (current_pos > line_start) {
        if (num_lines >= capacity) {
            capacity++;
            LineInfo *tmp = realloc(table, sizeof(LineInfo) * capacity);
            if (tmp == NULL) {
                perror("realloc");
                free(table);
                close(fd);
                return 1;
            }
            table = tmp;
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = (int)(current_pos - line_start);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    int line_no;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (scanf("%d", &line_no) != 1)
            break;

        if (line_no == 0)
            break;

        if (line_no < 1 || line_no > num_lines) {
            printf("Invalid line number (1..%d)\n", num_lines);
            continue;
        }

        LineInfo info = table[line_no - 1];

        if (lseek(fd, info.offset, SEEK_SET) == -1) {
            perror("lseek");
            continue;
        }

        char *buf = malloc(info.length + 1);
        if (buf == NULL) {
            perror("malloc");
            break;
        }

        ssize_t got = read(fd, buf, info.length);
        if (got == -1) {
            perror("read");
            free(buf);
            continue;
        }
        buf[got] = '\0';

        printf("%s\n", buf);
        free(buf);
    }

    free(table);
    close(fd);
    return 0;
}
