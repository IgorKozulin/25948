#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

typedef struct {
    long offset;
    int  length;
} LineInfo;

static int    g_fd = -1;
static char  *g_filename = NULL;

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

static void alarm_handler(int sig)
{
    (void)sig;

    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    int fd = open(g_filename, O_RDONLY);
    if (fd == -1) {
        _exit(EXIT_FAILURE);
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        ssize_t written = 0;
        while (written < n) {
            ssize_t w = write(STDOUT_FILENO, buf + written, n - written);
            if (w <= 0) break;
            written += w;
        }
    }

    close(fd);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    g_filename = argv[1];
    g_fd = open(g_filename, O_RDONLY);
    if (g_fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    LineInfo *table = NULL;
    int count = 0;
    build_table(g_fd, &table, &count);
    print_table(table, count);

    signal(SIGALRM, alarm_handler);

    int number;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &number) != 1) {
            alarm(0);
            break;
        }

        alarm(0);

        if (number == 0) {
            break;
        }
        if (number < 1 || number > count) {
            printf("Line number out of range (1..%d)\n", count);
            continue;
        }

        print_line(g_fd, &table[number - 1]);
    }

    free(table);
    close(g_fd);
    return 0;
}
