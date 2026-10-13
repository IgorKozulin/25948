#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

char *global_map = NULL;
size_t global_size = 0;

void alarm_handler(int sig) {
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    
    if (global_map != NULL && global_size > 0) {
        write(STDOUT_FILENO, global_map, global_size);
        if (global_map[global_size - 1] != '\n') {
            write(STDOUT_FILENO, "\n", 1);
        }
    }
    _exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
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

    global_size = (size_t)st.st_size;

    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    size_t capacity = 16;
    size_t num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) {
        perror("malloc");
        munmap(global_map, global_size);
        return 1;
    }

    off_t line_start = 0;
    for (size_t i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *new_table = realloc(table, capacity * sizeof(LineInfo));
                if (!new_table) {
                    perror("realloc");
                    free(table);
                    munmap(global_map, global_size);
                    return 1;
                }
                table = new_table;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start + 1;
            num_lines++;
            line_start = i + 1;
        }
    }

    if (line_start < (off_t)global_size) {
        if (num_lines >= capacity) {
            capacity += 1;
            LineInfo *new_table = realloc(table, capacity * sizeof(LineInfo));
            if (!new_table) {
                perror("realloc");
                free(table);
                munmap(global_map, global_size);
                return 1;
            }
            table = new_table;
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = global_size - line_start;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (size_t i = 0; i < num_lines; i++) {
        printf("Line %zu: Offset = %lld, Length = %zu\n",
               i + 1, (long long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int line_num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);
        int scan_res = scanf("%d", &line_num);
        alarm(0);

        if (scan_res != 1) {
            if (feof(stdin)) {
                printf("\n");
                break;
            }
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (line_num == 0) {
            break;
        }

        if (line_num < 0 || (size_t)line_num > num_lines) {
            printf("Invalid line number. Choose between 1 and %zu.\n", num_lines);
            continue;
        }

        LineInfo info = table[line_num - 1];
        fwrite(global_map + info.offset, 1, info.length, stdout);

        if (global_map[info.offset + info.length - 1] != '\n') {
            putchar('\n');
        }
        fflush(stdout);
    }

    free(table);
    if (munmap(global_map, global_size) == -1) {
        perror("munmap");
        return 1;
    }

    return 0;
}
