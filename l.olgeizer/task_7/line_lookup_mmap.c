#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/mman.h>

struct { off_t off; int len; } tbl[1024];

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) return 1;

    struct stat st;
    fstat(fd, &st);
    if (st.st_size == 0) return 0;
    off_t size = st.st_size;

    char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

    /* 1. Таблица строк */
    int n = 0;
    off_t start = 0, total = 0;
    for (off_t i = 0; i < size; i++) {
        if (map[i] == '\n') {
            tbl[n].off = start;
            tbl[n++].len = i - start + 1;
            start = i + 1;
        }
    }
    if (start < size) {
        tbl[n].off = start;
        tbl[n++].len = size - start;
    }

    /* 2. Вывод таблицы с размерами */
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < n; i++) {
        printf("Line %d: Offset = %ld, Length = %d, Size = %d\n",
               i + 1, (long)tbl[i].off, tbl[i].len, tbl[i].len);
        total += tbl[i].len;
    }
    printf("Total file size: %ld bytes\n-------------------------\n", (long)total);

    /* 3. Опрос с таймаутом 5 секунд (5000 мс) */
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        struct pollfd pfd = { 0, POLLIN, 0 };
        if (poll(&pfd, 1, 5000) == 0) {
            printf("\nTimeout (5 sec)! Printing whole file:\n");
            write(1, map, size);
            break;
        }

        int req;
        if (scanf("%d", &req) != 1 || req == 0) break;
        if (req > 0 && req <= n) {
            write(1, map + tbl[req - 1].off, tbl[req - 1].len);
        }
    }

    munmap(map, size);
    close(fd);
    return 0;
}
