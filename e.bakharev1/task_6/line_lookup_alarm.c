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

char g_filename[256];

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

void alarm_handler(int sig) {
    int fd = open(g_filename, O_RDONLY);
    if (fd != -1) {
        char buf[1024];
        ssize_t bytes_read;
        printf("\n[TIMEOUT] Time is up! Printing full file content...\n");
        while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
        }
        close(fd);
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    strncpy(g_filename, argv[1], sizeof(g_filename) - 1);
    g_filename[sizeof(g_filename) - 1] = '\0';

    int fd = open(g_filename, O_RDONLY);
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

    signal(SIGALRM, alarm_handler);

    int line_num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &line_num) != 1) {
            alarm(0);
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Try again.\n");
            continue;
        }

        alarm(0);

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
