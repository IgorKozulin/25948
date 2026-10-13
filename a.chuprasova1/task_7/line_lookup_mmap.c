#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct {
    long offset;
    int  length;
} LineInfo;

static char *g_map  = NULL;   // mmap-адреc
static size_t g_size = 0;     // размер файла

static void on_alarm(int sig)
{
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    size_t off = 0;
    while (off < g_size) {
        ssize_t w = write(STDOUT_FILENO, g_map + off, g_size - off);
        if (w < 0) _exit(EXIT_FAILURE);
        off += (size_t)w;
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { perror("open"); return EXIT_FAILURE; }

    struct stat st;
    if (fstat(fd, &st) < 0) {
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

    g_map = mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    LineInfo *table = NULL;
    size_t num_lines = 0, capacity = 0;
    long line_pos = 0;

    for (size_t i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') {
            if (num_lines == capacity) {
                capacity = capacity ? capacity * 2 : 16;
                LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
                if (!tmp) { perror("realloc"); free(table); munmap(g_map, g_size); return EXIT_FAILURE; }
                table = tmp;
            }
            table[num_lines].offset = line_pos;
            table[num_lines].length = (int)((long)i - line_pos);
            num_lines++;
            line_pos = (long)i + 1;
        }
    }
    // Последняя строка без '\n'
    if (line_pos < (long)g_size) {
        if (num_lines == capacity) {
            capacity = capacity ? capacity * 2 : 16;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
            if (!tmp) { perror("realloc"); free(table); munmap(g_map, g_size); return EXIT_FAILURE; }
            table = tmp;
        }
        table[num_lines].offset = line_pos;
        table[num_lines].length = (int)((long)g_size - line_pos);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (size_t i = 0; i < num_lines; i++) {
        printf("Line %zu: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");
    printf("Total lines: %zu\n\n", num_lines);

    signal(SIGALRM, on_alarm);

    char inbuf[64];
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) {
            alarm(0);
            break;
        }
        alarm(0);

        long num = strtol(inbuf, NULL, 10);
        if (num == 0) break;
        if (num < 1 || (size_t)num > num_lines) {
            printf("Out of range (1..%zu)\n", num_lines);
            continue;
        }

        LineInfo *li = &table[num - 1];
        
        fwrite(g_map + li->offset, 1, li->length, stdout);
        fputc('\n', stdout);
    }

    free(table);
    munmap(g_map, g_size);
    return 0;
}
