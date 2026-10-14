#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

//для хранения смещения и длины строки 
typedef struct {
    off_t  offset;
    size_t length;
} LineInfo;

// глобальные, чтобы обработчик сигнала имел к ним доступ
static int   g_fd = -1;
static char *g_filename = NULL;

//глобальные, чтобы main и print_line видели 
static LineInfo *g_table = NULL;
static int       g_lines_count = 0;

// печатает весь файл и выходит
void alarm_handler(int sig)
{
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    int fd = open(g_filename, O_RDONLY);
    if (fd >= 0) {
        char buf[4096];
        ssize_t n;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, n);
        }
        close(fd);
    }
    _exit(EXIT_SUCCESS);
}

// построение таблицы строк
void build_table(void)
{
    g_fd = open(g_filename, O_RDONLY);
    if (g_fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // считаем количество строк
    int count = 0;
    char c;
    while (read(g_fd, &c, 1) == 1) {
        if (c == '\n') count++;
    }
    if (count == 0) count = 1;

    g_table = malloc(sizeof(LineInfo) * count);
    if (!g_table) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // заполняем таблицу
    lseek(g_fd, 0, SEEK_SET);
    off_t offset = 0;
    int   idx    = 0;
    while (read(g_fd, &c, 1) == 1) {
        if (c == '\n') {
            g_table[idx].offset = offset;
            g_table[idx].length = lseek(g_fd, 0, SEEK_CUR) - offset;
            offset = lseek(g_fd, 0, SEEK_CUR);
            idx++;
        }
    }
    // последняя строка без \n
    off_t end = lseek(g_fd, 0, SEEK_END);
    if (offset < end) {
        g_table[idx].offset = offset;
        g_table[idx].length = end - offset;
        idx++;
    }
    g_lines_count = idx;

    // отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < g_lines_count; i++) {
        printf("Line %d: Offset = %ld, Length = %zu\n",
               i + 1, (long)g_table[i].offset, g_table[i].length);
    }
    printf("-------------------------\n");
}

// вывод строки по номеру
void print_line(int n)
{
    if (n < 1 || n > g_lines_count) {
        printf("Line %d out of range (1..%d).\n", n, g_lines_count);
        return;
    }
    LineInfo li = g_table[n - 1];
    char *buf = malloc(li.length + 1);
    if (!buf) { perror("malloc"); return; }

    lseek(g_fd, li.offset, SEEK_SET);
    ssize_t r = read(g_fd, buf, li.length);
    if (r > 0) {
        buf[r] = '\0';
        printf("%s", buf);
        if (buf[r - 1] != '\n') printf("\n");
    }
    free(buf);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    g_filename = argv[1];

    //построение таблицы строк
    build_table();                          

    //обработчик SIGALRM
    signal(SIGALRM, alarm_handler);

    int line_no;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);   // запуск таймера

        if (scanf("%d", &line_no) != 1) {
            alarm(0);        //отмена таймера, если не число
            // очистка буфера
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input.\n");
            continue;
        }

        alarm(0);                 //успел ввести — отменяем таймер

        if (line_no == 0) break;

        //вывод строки line_no через lseek + read
        print_line(line_no);
    }

    close(g_fd);
    free(g_table);
    return EXIT_SUCCESS;
}
