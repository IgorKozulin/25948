#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

char g_filename[256];
char *g_map = NULL;
size_t g_size = 0;

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
    printf("\n[TIMEOUT] Time is up! Printing full file content...\n");
    write(STDOUT_FILENO, g_map, g_size);
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

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    g_size = st.st_size;
    if (g_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    g_map = mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    LineInfo *table = NULL;
    int num_lines = 0;
    int capacity = 0;

    long line_start = 0;
    int current_len = 0;

    for (size_t i = 0; i < g_size; i++) {
        current_len++;
        if (g_map[i] == '\n') {
            add_line(&table, &num_lines, &capacity, line_start, current_len);
            line_start = i + 1;
            current_len = 0;
        }
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
        fwrite(g_map + info.offset, 1, info.length, stdout);
        if (info.length > 0 && g_map[info.offset + info.length - 1] != '\n') {
            printf("\n");
        }
    }

    free(table);
    munmap(g_map, g_size);
    return 0;
}
