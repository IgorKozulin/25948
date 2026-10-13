#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

typedef struct { long offset; int length; } LineInfo;

static const char *g_name;
static LineInfo *g_tab;
static int g_cnt, g_cap;

static void add_line(long off, int len)
{
    if (g_cnt == g_cap) {
        g_cap = g_cap ? g_cap * 2 : 16;
        g_tab = realloc(g_tab, (size_t)g_cap * sizeof(LineInfo));
        if (g_tab == NULL) { perror("realloc"); _exit(1); }
    }
    g_tab[g_cnt].offset = off;
    g_tab[g_cnt].length = len;
    g_cnt++;
}

/* handler: only async-signal-safe calls */
static void alarm_handler(int sig)
{
    char buf[4096];
    ssize_t n, w, k;
    int fd;
    (void)sig;
    (void)write(1, "\n[TIMEOUT] Time is up! Full file content:\n", 43);
    fd = open(g_name, O_RDONLY);
    if (fd < 0) { _exit(1); }
    while ((n = read(fd, buf, sizeof buf)) > 0) {
        w = 0;
        while (w < n) {
            k = write(1, buf + w, (size_t)(n - w));
            if (k <= 0) { break; }
            w += k;
        }
    }
    close(fd);
    _exit(0);
}
int main(int argc, char **argv)
{
    long line_start = 0, fsize;
    char c;
    ssize_t n;
    int fd, num, i;

    if (argc != 2) { fprintf(stderr, "Usage: %s <file>\n", argv[0]); return 1; }
    g_name = argv[1];
    fd = open(g_name, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    while ((n = read(fd, &c, 1)) == 1) {
        long cur = lseek(fd, 0L, SEEK_CUR);
        if (c == '\n') {
            if (cur - line_start > 1) { add_line(line_start, (int)(cur - line_start)); }
            line_start = cur;
        }
    }
    fsize = lseek(fd, 0L, SEEK_CUR);
    if (fsize > line_start) { add_line(line_start, (int)(fsize - line_start)); }

    printf("--- Debug: Line Table ---\n");
    for (i = 0; i < g_cnt; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, g_tab[i].offset, g_tab[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);
    for (;;) {
        int rc, saved, ch;
        LineInfo *li;
        char *buf;
        ssize_t rd;

        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);
        alarm(5);
        rc = scanf("%d", &num);
        saved = errno;
        alarm(0);

        if (rc == EOF) { break; }
        if (rc == 0) {
            if (saved == EINTR) { break; }
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            continue;
        }
        if (num == 0) { break; }
        if (num < 1 || num > g_cnt) { printf("No such line: %d\n", num); continue; }

        li = &g_tab[num - 1];
        buf = malloc((size_t)li->length + 1);
        if (buf == NULL) { perror("malloc"); break; }
        lseek(fd, li->offset, SEEK_SET);
        rd = read(fd, buf, (size_t)li->length);
        if (rd < 0) { perror("read"); free(buf); break; }
        buf[rd] = '\0';
        if (rd > 0 && buf[rd - 1] == '\n') { buf[rd - 1] = '\0'; }
        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    free(g_tab);
    return 0;
}
