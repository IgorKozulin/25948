#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    LineInfo *table = NULL;
    int num_lines = 0, capacity = 0;
    long current_pos = 0, line_start = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        current_pos++;
        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity ? capacity * 2 : 16;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (!table) { perror("realloc"); close(fd); return 1; }
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_pos - line_start - 1;
            num_lines++;
            line_start = current_pos;
        }
    }

    if (line_start < current_pos) {
        if (num_lines >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            table = realloc(table, capacity * sizeof(LineInfo));
            if (!table) { perror("realloc"); close(fd); return 1; }
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_pos - line_start;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++)
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);

    int n;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);
        if (scanf("%d", &n) != 1) break;
        if (n == 0) break;
        if (n < 1 || n > num_lines) {
            printf("Invalid line number\n");
            continue;
        }
        int idx = n - 1;
        lseek(fd, table[idx].offset, SEEK_SET);
        char *buf = malloc(table[idx].length + 1);
        if (!buf) { perror("malloc"); break; }
        read(fd, buf, table[idx].length);
        buf[table[idx].length] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(table);
    return 0;
}
