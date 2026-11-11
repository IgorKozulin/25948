#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct { long offset; int length; } LineInfo;

static char *g_map;
static size_t g_size;
static LineInfo *g_table;
static int g_n;

static void alarm_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, g_map, g_size);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Usage: %s file\n", argv[0]); return 1; }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) != 0) { perror("fstat"); close(fd); return 1; }
    g_size = st.st_size;
    if (g_size == 0) { printf("File is empty.\n"); close(fd); return 0; }

    g_map = mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) { perror("mmap"); close(fd); return 1; }
    close(fd);

    int cap = 0;
    long line_start = 0;
    for (size_t i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') {
            if (g_n == cap) {
                cap = cap ? cap * 2 : 16;
                g_table = realloc(g_table, cap * sizeof(LineInfo));
            }
            g_table[g_n].offset = line_start;
            g_table[g_n].length = (int)(i - line_start);
            g_n++;
            line_start = i + 1;
        }
    }
    if ((size_t)line_start < g_size) {
        if (g_n == cap) {
            cap = cap ? cap * 2 : 16;
            g_table = realloc(g_table, cap * sizeof(LineInfo));
        }
        g_table[g_n].offset = line_start;
        g_table[g_n].length = (int)(g_size - line_start);
        g_n++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < g_n; i++)
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, g_table[i].offset, g_table[i].length);
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);
        alarm(5);
        int rc = scanf("%d", &num);
        alarm(0);
        if (rc != 1) { while (getchar() != '\n'); continue; }
        if (num == 0) break;
        if (num < 1 || num > g_n) { printf("Out of range\n"); continue; }

        int idx = num - 1;
        fwrite(g_map + g_table[idx].offset, 1, g_table[idx].length, stdout);
        printf("\n");
    }

    free(g_table);
    munmap(g_map, g_size);
    return 0;
}