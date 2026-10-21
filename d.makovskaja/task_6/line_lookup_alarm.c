#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

int global_fd;

typedef struct {
    long offset;
    int length;
} LineInfo;

void alarm_handler(int sig) {
    printf("\n[TIMEOUT] Time is up! Printing full file content...\n");
    lseek(global_fd, 0, SEEK_SET);
    char buf[1024];
    ssize_t bytes;
    while ((bytes = read(global_fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, bytes);
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    global_fd = open(argv[1], O_RDONLY);
    if (global_fd == -1) {
        perror("Error opening file");
        return 1;
    }

    int num_lines = 0;
    int capacity = 10;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));

    char ch;
    long current_pos = 0;
    long line_start = 0;

    while (read(global_fd, &ch, 1) == 1) {
        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_pos - line_start;
            num_lines++;
            line_start = current_pos + 1;
        }
        current_pos++;
    }

    if (current_pos > line_start) {
        if (num_lines >= capacity) {
            capacity *= 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_pos - line_start;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        int line_num;
        if (scanf("%d", &line_num) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            alarm(0);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        alarm(0);

        if (line_num == 0) break;

        if (line_num < 1 || line_num > num_lines) {
            printf("Line number out of range (1..%d)\n", num_lines);
            continue;
        }

        long offset = table[line_num - 1].offset;
        int length = table[line_num - 1].length;

        lseek(global_fd, offset, SEEK_SET);

        char *buffer = (char *)malloc(length + 1);
        read(global_fd, buffer, length);
        buffer[length] = '\0';

        printf("%s\n", buffer);
        free(buffer);
    }

    free(table);
    close(global_fd);
    return 0;
}
