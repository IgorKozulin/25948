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

static char *g_map = NULL;
static size_t g_size = 0;

void alarm_handler(int sig)
{
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (g_map != NULL && g_size > 0) {
        ssize_t written = write(STDOUT_FILENO, g_map, g_size);
        (void)written;
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
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

    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    size_t size = (size_t)st.st_size;

    char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    g_map = map;
    g_size = size;

    LineInfo *lines = NULL;
    int num_lines = 0;
    int capacity = 0;

    long line_start = 0;

    for (size_t i = 0; i < size; i++) {
        if (map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity = capacity ? capacity * 2 : 16;
                lines = realloc(lines, capacity * sizeof(LineInfo));
                if (lines == NULL) {
                    perror("realloc");
                    munmap(map, size);
                    return 1;
                }
            }
            lines[num_lines].offset = line_start;
            lines[num_lines].length = (int)(i - line_start + 1);
            num_lines++;
            line_start = (long)(i + 1);
        }
    }

    if (line_start < (long)size) {
        if (num_lines >= capacity) {
            capacity = capacity ? capacity * 2 : 16;
            lines = realloc(lines, capacity * sizeof(LineInfo));
            if (lines == NULL) {
                perror("realloc");
                munmap(map, size);
                return 1;
            }
        }
        lines[num_lines].offset = line_start;
        lines[num_lines].length = (int)((long)size - line_start);
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, lines[i].offset, lines[i].length);
    }
    printf("-------------------------\n");

    printf("File size:  %ld bytes\n", (long)size);
    printf("Table size: %d lines x %d bytes = %d bytes\n",
           num_lines, (int)sizeof(LineInfo),
           num_lines * (int)sizeof(LineInfo));
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &num) != 1) {
            alarm(0);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard */
            }
            printf("Invalid input, try again.\n");
            continue;
        }

        alarm(0);

        if (num == 0)
            break;

        if (num < 1 || num > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        int idx = num - 1;
        fwrite(map + lines[idx].offset, 1, lines[idx].length, stdout);
        if (lines[idx].length > 0 && map[lines[idx].offset + lines[idx].length - 1] != '\n')
            printf("\n");
    }

    free(lines);
    munmap(map, size);
    return 0;
}
