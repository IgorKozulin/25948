
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

static int g_fd = -1;

void alarm_handler(int sig)
{
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (lseek(g_fd, 0, SEEK_SET) < 0) {
        _exit(EXIT_FAILURE);
    }

    char buf[512];
    ssize_t n;
    while ((n = read(g_fd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            _exit(EXIT_FAILURE);
        }
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
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

    LineInfo *lines = NULL;
    int num_lines = 0;
    int capacity = 0;

    char ch;
    long pos = 0;
    long line_start = 0;

    while (read(fd, &ch, 1) == 1) {
        pos++;
        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity ? capacity * 2 : 16;
                lines = realloc(lines, capacity * sizeof(LineInfo));
                if (lines == NULL) {
                    perror("realloc");
                    close(fd);
                    return 1;
                }
            }
            lines[num_lines].offset = line_start;
            lines[num_lines].length = pos - line_start;
            num_lines++;
            line_start = pos;
        }
    }

    if (pos > line_start) {
        if (num_lines >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            lines = realloc(lines, capacity * sizeof(LineInfo));
            if (lines == NULL) {
                perror("realloc");
                close(fd);
                return 1;
            }
        }
        lines[num_lines].offset = line_start;
        lines[num_lines].length = pos - line_start;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, lines[i].offset, lines[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &num) != 1) {
            alarm(0);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard */
            }
            printf("Invalid input, try again.\n");
            continue;
        }

        alarm(0);

        if (num == 0)
            break;

        if (num < 1 || num > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        int idx = num - 1;
        if (lseek(fd, lines[idx].offset, SEEK_SET) < 0) {
            perror("lseek");
            continue;
        }

        char *buf = malloc(lines[idx].length + 1);
        if (buf == NULL) {
            perror("malloc");
            continue;
        }

        ssize_t n = read(fd, buf, lines[idx].length);
        if (n < 0) {
            perror("read");
            free(buf);
            continue;
        }
        buf[n] = '\0';
        printf("%s", buf);
        if (n > 0 && buf[n - 1] != '\n')
            printf("\n");

        free(buf);
    }

    free(lines);
    close(fd);
    return 0;
}
