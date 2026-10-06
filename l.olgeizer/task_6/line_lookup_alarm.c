#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

static int g_fd = -1;

typedef struct {
    off_t offset;
    int length;
} LineInfo;

void alarm_handler(int sig) {
    (void)sig;
    const char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (g_fd != -1) {
        lseek(g_fd, 0, SEEK_SET);
        char buf[1024];
        ssize_t bytes;
        while ((bytes = read(g_fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes);
        }
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    g_fd = open(argv[1], O_RDONLY);
    if (g_fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    int capacity = 16, num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        close(g_fd);
        return EXIT_FAILURE;
    }

    off_t line_start = 0;
    char ch;

    while (read(g_fd, &ch, 1) > 0) {
        if (ch == '\n') {
            off_t next_line = lseek(g_fd, 0L, 1);
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (!temp) { perror("realloc"); free(table); close(g_fd); return 1; }
                table = temp;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(next_line - line_start);
            num_lines++;
            line_start = next_line;
        }
    }

    off_t file_end = lseek(g_fd, 0L, 1);
    if (file_end > line_start) {
        if (num_lines >= capacity) {
            capacity++;
            LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            if (temp) table = temp;
        }
        if (num_lines < capacity) {
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(file_end - line_start);
            num_lines++;
        }
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, (long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int req_line;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);
        int res = scanf("%d", &req_line);
        alarm(0);

        if (res != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Некорректный ввод. Введите число.\n");
            continue;
        }

        if (req_line == 0) {
            break;
        }

        if (req_line < 1 || req_line > num_lines) {
            fprintf(stderr, "Недопустимый номер строки. Диапазон: 1..%d\n", num_lines);
            continue;
        }

        LineInfo info = table[req_line - 1];
        if (lseek(g_fd, info.offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }

        char *buf = (char *)malloc(info.length + 1);
        if (!buf) { perror("malloc"); continue; }

        ssize_t bytes_read = read(g_fd, buf, info.length);
        if (bytes_read > 0) {
            buf[bytes_read] = '\0';
            printf("%s", buf);
            if (buf[bytes_read - 1] != '\n') {
                printf("\n");
            }
        }
        free(buf);
    }

    free(table);
    close(g_fd);
    return EXIT_SUCCESS;
}
