#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct { long offset; int length; } LineInfo;

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Usage: %s file\n", argv[0]); return 1; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    LineInfo *table = NULL;
    int n = 0, cap = 0;
    long line_start = 0, pos = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        pos++;
        if (ch == '\n') {
            if (n == cap) {
                cap = cap ? cap * 2 : 16;
                table = realloc(table, cap * sizeof(LineInfo));
            }
            table[n].offset = line_start;
            table[n].length = pos - line_start - 1;
            n++;
            line_start = pos;
        }
    }
    if (line_start < pos) {
        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            table = realloc(table, cap * sizeof(LineInfo));
        }
        table[n].offset = line_start;
        table[n].length = pos - line_start;
        n++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < n; i++)
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    printf("-------------------------\n");

    int num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);
        if (scanf("%d", &num) != 1) { while (getchar() != '\n'); continue; }
        if (num == 0) break;
        if (num < 1 || num > n) { printf("Out of range\n"); continue; }

        int idx = num - 1;
        lseek(fd, table[idx].offset, SEEK_SET);
        char *buf = malloc(table[idx].length + 1);
        read(fd, buf, table[idx].length);
        buf[table[idx].length] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(table);
    return 0;
}