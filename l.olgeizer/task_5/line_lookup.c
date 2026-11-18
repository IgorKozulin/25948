#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct { off_t off; int len; } tbl[1024];

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) return 1;

    int n = 0;
    off_t start = 0, total = 0;
    char c;

    /* Построение таблицы смещений */
    while (read(fd, &c, 1) > 0) {
        if (c == '\n') {
            off_t next = lseek(fd, 0, SEEK_CUR);
            tbl[n].off = start;
            tbl[n++].len = next - start;
            start = next;
        }
    }
    off_t end = lseek(fd, 0, SEEK_CUR);
    if (end > start) {
        tbl[n].off = start;
        tbl[n++].len = end - start;
    }

    /* Вывод таблицы и общего размера */
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < n; i++) {
        printf("Line %d: Offset = %ld, Length = %d, Size = %d\n", 
               i + 1, (long)tbl[i].off, tbl[i].len, tbl[i].len);
        total += tbl[i].len;
    }
    printf("Total file size: %ld bytes\n-------------------------\n", (long)total);

    /* Интерактивный поиск */
    int req;
    while (printf("Enter line number (0 to quit): ") && scanf("%d", &req) == 1 && req > 0) {
        if (req <= n) {
            char buf[1024];
            lseek(fd, tbl[req - 1].off, SEEK_SET);
            int r = read(fd, buf, tbl[req - 1].len);
            write(STDOUT_FILENO, buf, r);
            if (buf[r - 1] != '\n') write(STDOUT_FILENO, "\n", 1);
        }
    }

    close(fd);
    return 0;
}
