#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

static int g_fd = -1;

void alarm_handler(int sig) {
    (void)sig;
    printf("\n[TIMEOUT] 5 секунд истекли! Вывожу весь файл:\n");
    lseek(g_fd, 0L, SEEK_SET);
    char buf[4096];
    ssize_t n;
    while ((n = read(g_fd, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, n);
    _exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    g_fd = fd;

    LineInfo *table = NULL;
    int num_lines = 0, capacity = 0;
    long line_start = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        long current_pos = lseek(fd, 0L, SEEK_CUR);
        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity ? capacity * 2 : 16;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (!table) { perror("realloc"); close(fd); return 1; }
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(current_pos - line_start - 1);
            num_lines++;
            line_start = current_pos;
        }
    }

    long file_end = lseek(fd, 0L, SEEK_CUR);
    if (line_start < file_end) {
        if (num_lines >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            table = realloc(table, capacity * sizeof(LineInfo));
            if (!table) { perror("realloc"); close(fd); return 1; }
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = (int)(file_end - line_start);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int n;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);
        alarm(5);
        if (scanf("%d", &n) != 1) {
            alarm(0);
            break;
        }
        alarm(0);
        if (n == 0) break;
        if (n < 1 || n > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        int idx = n - 1;
        lseek(fd, table[idx].offset, SEEK_SET);

        char *buf = malloc(table[idx].length + 1);
        if (!buf) { perror("malloc"); break; }
        ssize_t got = read(fd, buf, table[idx].length);
        if (got < 0) { perror("read"); free(buf); break; }
        buf[got] = '\0';
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(table);
    return 0;
}
