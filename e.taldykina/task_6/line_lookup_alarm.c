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

int global_fd = -1;

void alarm_handler(int sig) {
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    
    if (global_fd != -1) {
        lseek(global_fd, 0, SEEK_SET);
        char buf[1024];
        ssize_t bytes;
        while ((bytes = read(global_fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes);
        }
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    global_fd = open(argv[1], O_RDONLY);
    if (global_fd == -1) {
        perror("open");
        return 1;
    }

    LineInfo *table = NULL;
    int num_lines = 0;
    int capacity = 0;
    long current_offset = 0;
    long line_start = 0;
    char c;

    while (read(global_fd, &c, 1) == 1) {
        if (c == '\n') {
            if (num_lines == capacity) {
                capacity = capacity == 0 ? 10 : capacity * 2;
                table = realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_offset - line_start + 1;
            num_lines++;
            line_start = current_offset + 1;
        }
        current_offset++;
    }

    if (current_offset > line_start) {
        if (num_lines == capacity) {
            capacity = capacity == 0 ? 10 : capacity * 2;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = current_offset - line_start;
        num_lines++;
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
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
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

        int idx = line_num - 1;
        lseek(global_fd, table[idx].offset, SEEK_SET);
        
        char *buffer = malloc(table[idx].length + 1);
        read(global_fd, buffer, table[idx].length);
        buffer[table[idx].length] = '\0';
        
        printf("%s", buffer);
        
        free(buffer);
    }

    free(table);
    close(global_fd);
    return 0;
}
