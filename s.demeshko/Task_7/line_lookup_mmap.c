#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

static char *g_map = NULL;
static long g_size = 0;

void alarm_handler(int sig) {
    (void)sig;
    printf("\n[TIMEOUT] 5 секунд истекли! Вывожу весь файл:\n");
    if (g_map && g_size > 0)
        write(STDOUT_FILENO, g_map, g_size);
    _exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
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
        return 1;
    }
    close(fd);

    LineInfo *table = NULL;
    int num_lines = 0, capacity = 0;
    long line_start = 0;

    for (long i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity ? capacity * 2 : 16;
                table = realloc(table, capacity * sizeof(LineInfo));
                if (!table) { perror("realloc"); munmap(g_map, g_size); return 1; }
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = (int)(i - line_start);
            num_lines++;
            line_start = i + 1;
        }
    }

    if (line_start < g_size) {
        if (num_lines >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            table = realloc(table, capacity * sizeof(LineInfo));
            if (!table) { perror("realloc"); munmap(g_map, g_size); return 1; }
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = (int)(g_size - line_start);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int n;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);
        alarm(5);
        if (scanf("%d", &n) != 1) {
            alarm(0);
            break;
        }
        alarm(0);
        if (n == 0) break;
        if (n < 1 || n > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        int idx = n - 1;
        fwrite(g_map + table[idx].offset, 1, table[idx].length, stdout);
        printf("\n");
    }

    free(table);
    munmap(g_map, g_size);
    return 0;
}
