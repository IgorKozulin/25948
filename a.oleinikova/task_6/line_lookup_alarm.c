#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    long offset;
    int len_str;
} LineInfo;

typedef struct {
    LineInfo *data;
    int count;
    int cap;
} LineTable;

static char *g_map;
static size_t g_size;

static int open_f(const char *path);
static void t_add(LineTable *t, long offset, int len_str);
static void build_t(char *map, size_t size, LineTable *t);
static void print_t(const LineTable *t);
static void print_line(const char *map, const LineInfo *li);
static void input(const LineTable *t);
static void free_t(LineTable *t);
static void on_alarm(int sig);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    int fd = open_f(argv[1]);
    if (fd < 0) return 1;

    struct stat st;
    if (fstat(fd, &st) != 0) {
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

    LineTable table = { NULL, 0, 0 };
    build_t(g_map, g_size, &table);
    print_t(&table);

    signal(SIGALRM, on_alarm);

    input(&table);

    free_t(&table);
    munmap(g_map, g_size);
    return 0;
}

static int open_f(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) perror("open");
    return fd;
}

static void t_add(LineTable *t, long offset, int len_str) {
    if (t->count == t->cap) {
        t->cap = t->cap ? t->cap * 2 : 16;
        t->data = realloc(t->data, t->cap * sizeof(LineInfo));
        if (!t->data) {
            perror("realloc");
            exit(1);
        }
    }
    t->data[t->count].offset = offset;
    t->data[t->count].len_str = len_str;
    t->count++;
}

static void build_t(char *map, size_t size, LineTable *t) {
    long line_start = 0;

    for (size_t i = 0; i < size; i++) {
        if (map[i] == '\n') {
            t_add(t, line_start, (int)(i - line_start));
            line_start = i + 1;
        }
    }
    if ((size_t)line_start < size) {
        t_add(t, line_start, (int)(size - line_start));
    }
}

static void print_t(const LineTable *t) {
    printf("--- Line Table ---\n");
    printf("=========================\n");
    for (int i = 0; i < t->count; i++) {
        printf("Line %d: Offset = %ld, Len_str = %d\n",
               i + 1, t->data[i].offset, t->data[i].len_str);
    }
    printf("=========================\n");
}

static void print_line(const char *map, const LineInfo *li) {
    fwrite(map + li->offset, 1, li->len_str, stdout);
    printf("\n");
}

static void on_alarm(int sig) {
    (void)sig;
    write(STDOUT_FILENO, g_map, g_size);
    _exit(0);
}

static void input(const LineTable *t) {
    int num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);
        int rc = scanf("%d", &num);
        alarm(0);

        if (rc != 1) {
            while (getchar() != '\n');
            continue;
        }
        if (num == 0) break;
        if (num < 1 || num > t->count) {
            printf("Out of range\n");
            continue;
        }
        print_line(g_map, &t->data[num - 1]);
    }
}

static void free_t(LineTable *t) {
    free(t->data);
    t->data = NULL;
    t->count = 0;
    t->cap = 0;
}