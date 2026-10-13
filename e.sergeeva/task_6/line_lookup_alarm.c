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

char *global_filename = NULL;

void alarm_handler(int sig) {
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    int fd = open(global_filename, O_RDONLY);
    if (fd >= 0) {
        char buf[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes_read);
        }
        close(fd);
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    global_filename = argv[1];

    int fd = open(global_filename, O_RDONLY);
    LineInfo *table = NULL;
    int capacity = 0;
    int num_lines = 0;
    long line_start = lseek(fd, 0, SEEK_CUR);
    char ch;

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            long current_pos = lseek(fd, 0, SEEK_CUR);
            if (num_lines == capacity) {
                capacity = (capacity == 0) ? 10 : capacity * 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = current_pos - line_start;
            num_lines++;
            line_start = current_pos;
        }
    }

    long final_pos = lseek(fd, 0, SEEK_CUR);
    if (final_pos > line_start) {
        if (num_lines == capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = final_pos - line_start;
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
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Неверный ввод. Пожалуйста, введите число.\n");
            continue;
        }

        alarm(0);

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Неверный номер строки. Доступно строк: %d\n", num_lines);
            continue;
        }

        int idx = line_num - 1;
        lseek(fd, table[idx].offset, SEEK_SET);
        
        char *buffer = (char *)malloc(table[idx].length + 1);
        read(fd, buffer, table[idx].length);
        buffer[table[idx].length] = '\0';
        printf("%s", buffer);
        free(buffer);
    }

    free(table);
    close(fd);
    return 0;
}
