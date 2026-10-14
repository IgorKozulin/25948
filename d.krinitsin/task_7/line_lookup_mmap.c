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
    int  length;
} LineInfo;

static char *g_map  = NULL;
static long  g_size = 0;

static void build_table(char *map, long size, LineInfo **table_out, int *count_out)
{
    LineInfo *table = NULL;
    int count = 0;
    int capacity = 0;
    long line_start = 0;

    for (long i = 0; i < size; i++) {
        if (map[i] == '\n') {
            if (count == capacity) {
                capacity = (capacity == 0) ? 16 : capacity * 2;
                LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
                if (tmp == NULL) {
                    perror("realloc");
                    free(table);
                    exit(EXIT_FAILURE);
                }
                table = tmp;
            }
            table[count].offset = line_start;
            table[count].length = (int)(i - line_start);
            count++;

            line_start = i + 1;
        }
    }

    if (line_start < size) {
        if (count == capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
            if (tmp == NULL) {
                perror("realloc");
                free(table);
                exit(EXIT_FAILURE);
            }
            table = tmp;
        }
        table[count].offset = line_start;
        table[count].length = (int)(size - line_start);
        count++;
    }

    *table_out = table;
    *count_out = count;
}

static void print_table(const LineInfo *table, int count)
{
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < count; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");
}

static void alarm_handler(int sig)
{
    (void)sig;

    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (g_map != NULL && g_size > 0) {
        long written = 0;
        while (written < g_size) {
            ssize_t w = write(STDOUT_FILENO, g_map + written, g_size - written);
            if (w <= 0) break;
            written += w;
        }
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
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

    g_size = (long)st.st_size;
    g_map = mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    LineInfo *table = NULL;
    int count = 0;
    build_table(g_map, g_size, &table, &count);
    print_table(table, count);

    signal(SIGALRM, alarm_handler);

    int number;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &number) != 1) {
            alarm(0);
            break;
        }

        alarm(0);

        if (number == 0) {
            break;
        }
        if (number < 1 || number > count) {
            printf("Line number out of range (1..%d)\n", count);
            continue;
        }

        LineInfo *info = &table[number - 1];
        fwrite(g_map + info->offset, 1, info->length, stdout);
        fputc('\n', stdout);
    }

    free(table);
    munmap(g_map, g_size);
    return 0;
}
