#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>             

typedef struct {
    long offset;
    int  length;
} LineInfo;

static const char *g_filename = NULL;  //имя файла 


static void on_alarm(int sig)
{
    (void)sig;

    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    int fd = open(g_filename, O_RDONLY);
    if (fd < 0) {
        _exit(EXIT_FAILURE);
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(STDOUT_FILENO, buf + off, n - off);
            if (w < 0) { close(fd); _exit(EXIT_FAILURE); }
            off += w;
        }
    }

    close(fd);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    g_filename = argv[1];        // имя для обработчика

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    LineInfo *table = NULL;
    size_t num_lines = 0;
    size_t capacity  = 0;

    char buf[4096];
    ssize_t n;
    long line_pos = 0;
    long visit = 0;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                if (num_lines == capacity) {
                    capacity = capacity ? capacity * 2 : 16;
                    LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
                    if (tmp == NULL) {
                        perror("realloc");
                        free(table);
                        close(fd);
                        return EXIT_FAILURE;
                    }
                    table = tmp;
                }
                table[num_lines].offset = line_pos;
                table[num_lines].length = (int)(visit - line_pos);
                num_lines++;
                line_pos = visit + 1;
            }
            visit++;
        }
    }

    if (n < 0) {
        perror("read");
        free(table);
        close(fd);
        return EXIT_FAILURE;
    }

    if (line_pos < visit) {
        if (num_lines == capacity) {
            capacity = capacity ? capacity * 2 : 16;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
            if (tmp == NULL) {
                perror("realloc");
                free(table);
                close(fd);
                return EXIT_FAILURE;
            }
            table = tmp;
        }
        table[num_lines].offset = line_pos;
        table[num_lines].length = (int)(visit - line_pos);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (size_t i = 0; i < num_lines; i++) {
        printf("Line %zu: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");
    printf("Total lines: %zu\n\n", num_lines);

    signal(SIGALRM, on_alarm);   // обработчик будильника

    char inbuf[64];
    while (1) {
        
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);                

        if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) {
            alarm(0);            // ввод прерван -
            break;
        }

        alarm(0);                

        long num = strtol(inbuf, NULL, 10);
        if (num == 0) {
            break;
        }
        if (num < 1 || (size_t)num > num_lines) {
            printf("Out of range (1..%zu)\n", num_lines);
            continue;
        }

        LineInfo *li = &table[num - 1];

        if (lseek(fd, li->offset, SEEK_SET) < 0) {
            perror("lseek");
            continue;
        }

        char *line = malloc(li->length + 1);
        if (line == NULL) {
            perror("malloc");
            break;
        }

        ssize_t got = read(fd, line, li->length);
        if (got < 0) {
            perror("read");
            free(line);
            continue;
        }
        line[got] = '\0';

        printf("%s\n", line);
        free(line);
    }

    free(table);
    close(fd);
    return 0;
}
