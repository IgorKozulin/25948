#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

char *global_map = MAP_FAILED;
size_t global_size = 0;

void alarm_handler(int sig) {
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    if (global_map != MAP_FAILED) {
        write(STDOUT_FILENO, global_map, global_size);
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    global_size = st.st_size;
    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    LineInfo *table = NULL;
    int num_lines = 0;
    int capacity = 0;
    long line_start = 0;

    for (size_t i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines == capacity) {
                capacity = capacity == 0 ? 10 : capacity * 2;
                table = realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start + 1;
            num_lines++;
            line_start = i + 1;
        }
    }

    if (global_size > line_start) {
        if (num_lines == capacity) {
            capacity = capacity == 0 ? 10 : capacity * 2;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = global_size - line_start;
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
        fwrite(global_map + table[idx].offset, 1, table[idx].length, stdout);
    }

    free(table);
    munmap(global_map, global_size);
    return 0;
}
