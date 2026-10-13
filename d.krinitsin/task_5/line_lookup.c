#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
    long offset;
    int  length;
} LineInfo;

static void build_table(int fd, LineInfo **table_out, int *count_out)
{
    LineInfo *table = NULL;
    int count = 0;
    int capacity = 0;

    char buf[4096];
    ssize_t n;
    long pos = 0;
    long line_start = 0;
    int  line_len = 0;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                if (count == capacity) {
                    capacity = (capacity == 0) ? 16 : capacity * 2;
                    LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
                    if (tmp == NULL) {
                        perror("realloc");
                        free(table);
                        exit(EXIT_FAILURE);
                    }
                    table = tmp;
                }
                table[count].offset = line_start;
                table[count].length = line_len;
                count++;

                line_start = pos + 1;
                line_len = 0;
            } else {
                line_len++;
            }
            pos++;
        }
    }

    if (n < 0) {
        perror("read");
        free(table);
        exit(EXIT_FAILURE);
    }

    /* последняя строка без '\n' */
    if (line_len > 0) {
        if (count == capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
            if (tmp == NULL) {
                perror("realloc");
                free(table);
                exit(EXIT_FAILURE);
            }
            table = tmp;
        }
        table[count].offset = line_start;
        table[count].length = line_len;
        count++;
    }

    *table_out = table;
    *count_out = count;
}

static void print_table(const LineInfo *table, int count)
{
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < count; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");
}

static void print_line(int fd, const LineInfo *info)
{
    if (lseek(fd, info->offset, SEEK_SET) == -1) {
        perror("lseek");
        return;
    }

    char *buf = malloc(info->length + 1);
    if (buf == NULL) {
        perror("malloc");
        return;
    }

    ssize_t got = read(fd, buf, info->length);
    if (got < 0) {
        perror("read");
        free(buf);
        return;
    }
    buf[got] = '\0';

    printf("%s\n", buf);
    free(buf);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    LineInfo *table = NULL;
    int count = 0;
    build_table(fd, &table, &count);
    print_table(table, count);

    int number;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (scanf("%d", &number) != 1) {
            break;
        }
        if (number == 0) {
            break;
        }
        if (number < 1 || number > count) {
            printf("Line number out of range (1..%d)\n", count);
            continue;
        }

        print_line(fd, &table[number - 1]);
    }

    free(table);
    close(fd);
    return 0;
}
