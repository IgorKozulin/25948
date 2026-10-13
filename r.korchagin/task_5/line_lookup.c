#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    long offset;   /* позиция начала строки от начала файла */
    int  length;   /* длина строки в байтах (включая '\n') */
} LineInfo;

static void add_line(LineInfo **table, int *count, int *capacity,
                     long offset, int length)
{
    if (*count == *capacity) {
        *capacity *= 2;
        LineInfo *tmp = realloc(*table, (size_t)*capacity * sizeof(LineInfo));
        if (tmp == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        *table = tmp;
    }
    (*table)[*count].offset = offset;
    (*table)[*count].length = length;
    (*count)++;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <text-file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    int count = 0, capacity = 16;
    LineInfo *table = malloc((size_t)capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        close(fd);
        return 1;
    }

    long line_start = 0;
    char c;
    ssize_t n;
    while ((n = read(fd, &c, 1)) == 1) {
        long cur = lseek(fd, 0L, 1);   /* позиция после прочитанного байта */
        if (cur == -1) {
            perror("lseek");
            free(table);
            close(fd);
            return 1;
        }
        if (c == '\n') {
            /* пустая строка (length == 1, один '\n') в таблицу НЕ попадает */
            if (cur - line_start > 1) {
                add_line(&table, &count, &capacity,
                         line_start, (int)(cur - line_start));
            }
            line_start = cur;
        }
    }
    if (n == -1) {
        perror("read");
        free(table);
        close(fd);
        return 1;
    }

    /* последняя строка без '\n' (пустой быть не может) */
    long file_size = lseek(fd, 0L, 1);
    if (file_size > line_start) {
        add_line(&table, &count, &capacity,
                 line_start, (int)(file_size - line_start));
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < count; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    int num;
    for (;;) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &num) != 1) {
            fprintf(stderr, "Invalid input\n");
            break;
        }
        if (num == 0) {
            break;
        }
        if (num < 1 || num > count) {
            printf("No such line: %d (file has %d line(s))\n", num, count);
            continue;
        }

        LineInfo *li = &table[num - 1];
        if (lseek(fd, li->offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            break;
        }

        char *buf = malloc((size_t)li->length + 1);
        if (buf == NULL) {
            perror("malloc");
            break;
        }
        ssize_t rd = read(fd, buf, (size_t)li->length);
        if (rd == -1) {
            perror("read");
            free(buf);
            break;
        }
        buf[rd] = '\0';
        if (rd > 0 && buf[rd - 1] == '\n') {
            buf[rd - 1] = '\0';
        }
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(table);
    return 0;
}
