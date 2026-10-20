#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

void add_line(LineInfo **table, int *num_lines, int *capacity, long offset, int length) {
    if (*num_lines == *capacity) {
        *capacity = (*capacity == 0) ? 10 : (*capacity) * 2;
        *table = realloc(*table, (*capacity) * sizeof(LineInfo));
        if (*table == NULL) {
            exit(EXIT_FAILURE);
        }
    }
    (*table)[*num_lines].offset = offset;
    (*table)[*num_lines].length = length;
    (*num_lines)++;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    LineInfo *table = NULL;
    int num_lines = 0;
    int capacity = 0;

    char ch;
    long current_offset = 0;
    long line_start = 0;
    int current_len = 0;

    while (read(fd, &ch, 1) == 1) {
        current_len++;
        if (ch == '\n') {
            add_line(&table, &num_lines, &capacity, line_start, current_len);
            line_start = current_offset + 1;
            current_len = 0;
        }
        current_offset++;
    }

    if (current_len > 0) {
        add_line(&table, &num_lines, &capacity, line_start, current_len);
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            break;
        }

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        LineInfo info = table[line_num - 1];
        lseek(fd, info.offset, SEEK_SET);

        char *buf = malloc(info.length + 1);
        if (buf == NULL) {
            exit(EXIT_FAILURE);
        }

        read(fd, buf, info.length);
        buf[info.length] = '\0';

        printf("%s", buf);
        if (info.length > 0 && buf[info.length - 1] != '\n') {
            printf("\n");
        }

        free(buf);
    }

    free(table);
    close(fd);
    return 0;
}
