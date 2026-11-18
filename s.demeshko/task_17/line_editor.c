/* Задача 17: строчный редактор — raw mode, ERASE, KILL, CTRL-W, перенос по 40 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_BUF    4096
#define LINE_WIDTH 40          /* ширина строки в символах текста, без приглашения */

static struct termios saved_tio;
static int raw_active = 0;
static int rows_used  = 1;      /* сколько экранных строк заняла прошлая отрисовка */
static unsigned char erase_ch = 0x7f;
static unsigned char kill_ch  = 0x15;

static void restore_terminal(void)
{
    if (raw_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_tio);
        raw_active = 0;
    }
}

static void die(const char *what)
{
    restore_terminal();
    perror(what);
    exit(EXIT_FAILURE);
}

static void enable_raw_mode(void)
{
    struct termios t;

    if (tcgetattr(STDIN_FILENO, &saved_tio) == -1)
        die("tcgetattr");

    t = saved_tio;
    /* ISIG тоже выключаем: Ctrl-C должен пищать, а не убивать процесс */
    t.c_lflag &= ~(tcflag_t)(ECHO | ICANON | IEXTEN | ISIG);
    /* Enter должен приходить как '\r', Ctrl-S/Q не должны замораживать вывод */
    t.c_iflag &= ~(tcflag_t)(IXON | ICRNL | INLCR | BRKINT);
    t.c_cc[VMIN]  = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) == -1)
        die("tcsetattr");

    raw_active = 1;
    atexit(restore_terminal);

    erase_ch = saved_tio.c_cc[VERASE];
    kill_ch  = saved_tio.c_cc[VKILL];
}

/* Рисует текст с переносом слов по LINE_WIDTH, возвращает число экранных строк */
static int render(const char *buf, int len)
{
    int rows = 1, col = 0, i = 0;

    while (i < len) {
        if (buf[i] == ' ') {
            if (col == LINE_WIDTH) { putchar('\n'); rows++; col = 0; }
            putchar(' ');
            col++;
            i++;
            continue;
        }

        int w = 0;                                  /* длина слова */
        while (i + w < len && buf[i + w] != ' ') w++;

        /* слово не помещается в остаток строки — переносим целиком */
        if (col > 0 && col + w > LINE_WIDTH) {
            putchar('\n');
            rows++;
            col = 0;
        }

        for (int k = 0; k < w; k++) {                /* слово длиннее строки — режем */
            if (col == LINE_WIDTH) { putchar('\n'); rows++; col = 0; }
            putchar(buf[i + k]);
            col++;
        }
        i += w;
    }
    return rows;
}

static void redraw(const char *buf, int len)
{
    if (rows_used > 1)
        printf("\033[%dA", rows_used - 1);   /* вверх к первой строке ввода */
    printf("\r\033[J> ");                    /* в начало, стереть до конца экрана */
    rows_used = render(buf, len);
    fflush(stdout);
}

int main(void)
{
    char buf[MAX_BUF];
    int len = 0;
    unsigned char c;
    ssize_t n;

    enable_raw_mode();

    printf("> ");
    fflush(stdout);

    for (;;) {
        n = read(STDIN_FILENO, &c, 1);
        if (n == -1) {
            if (errno == EINTR) continue;
            die("read");
        }
        if (n == 0) continue;

        if (c == 4) {                         /* CTRL-D */
            if (len == 0) {
                printf("\r\nExiting...\n");
                fflush(stdout);
                break;                        /* atexit восстановит терминал */
            }
            putchar('\a');
            fflush(stdout);
            continue;
        }

        if (c == erase_ch || c == 8 || c == 0x7f) {    /* ERASE (Backspace) */
            if (len > 0) len--;
            else { putchar('\a'); fflush(stdout); continue; }
            redraw(buf, len);
            continue;
        }

        if (c == kill_ch) {                   /* KILL (Ctrl-U) — стереть всю строку */
            len = 0;
            redraw(buf, len);
            continue;
        }

        if (c == 23) {                        /* CTRL-W — слово + пробелы за ним */
            int i = len;
            while (i > 0 && isspace((unsigned char)buf[i - 1])) i--;
            while (i > 0 && !isspace((unsigned char)buf[i - 1])) i--;
            len = i;
            redraw(buf, len);
            continue;
        }

        if (c == '\r' || c == '\n') {         /* Enter — принять строку */
            putchar('\n');
            fflush(stdout);
            len = 0;
            rows_used = 1;
            printf("> ");
            fflush(stdout);
            continue;
        }

        if (isprint(c)) {
            if (len < MAX_BUF - 1) {
                buf[len++] = (char)c;
                redraw(buf, len);
            } else {
                putchar('\a');
                fflush(stdout);
            }
            continue;
        }

        putchar('\a');                        /* всё прочее (Ctrl-C, Tab, ...) — писк */
        fflush(stdout);
    }

    return 0;
}
