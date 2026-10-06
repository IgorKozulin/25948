#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

static char *g_map = MAP_FAILED;
static size_t g_size = 0;

typedef struct {
    off_t offset;
    int length;
} LineInfo;

void alarm_handler(int sig) {
    (void)sig;
    const char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (g_map != MAP_FAILED && g_size > 0) {
        write(STDOUT_FILENO, g_map, g_size);
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    g_size = (size_t)st.st_size;

    g_map = (char *)mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    int capacity = 16, num_lines = 0;
    LineInfo *table = (LineInfo *)malloc(capacity * sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        munmap(g_map, g_size);
        return EXIT_FAILURE;
    }

    off_t line_start = 0;
    for (size_t i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
                if (!temp) {
                    perror("realloc");
                    free(table);
                    munmap(g_map, g_size);
                    return EXIT_FAILURE;
                }
                table = temp;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(i - line_start + 1);
            num_lines++;
            line_start = (off_t)(i + 1);
        }
    }

    if ((size_t)line_start < g_size) {
        if (num_lines >= capacity) {
            capacity++;
            LineInfo *temp = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            if (temp) table = temp;
        }
        if (num_lines < capacity) {
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(g_size - line_start);
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

        fwrite(g_map + info.offset, 1, info.length, stdout);
        if (g_map[info.offset + info.length - 1] != '\n') {
            printf("\n");
        }
    }

    free(table);
    munmap(g_map, g_size);

    return EXIT_SUCCESS;
}
