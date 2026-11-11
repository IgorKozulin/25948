#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LINE 4096
#define WRAP_COL 40

static struct termios orig_termios;
static int prev_lines = 1;

// Восстановление терминала при выходе
static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Включение raw-режима (отключаем эхо и канонику)
static void enable_raw_mode(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    atexit(disable_raw_mode);

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Перерисовка строки с учетом переноса (40 символов)
static void redraw(const char *buf, int len)
{
    // Поднимаем курсор, чтобы стереть предыдущий вывод
    for (int i = 1; i < prev_lines; i++) {
        printf("\033[A");
    }
    printf("\r\033[J");

    printf("> ");
    int col = 2;
    int line_count = 1;
    int i = 0;

    while (i < len) {
        if (buf[i] == ' ') {
            if (col >= WRAP_COL) {
                printf("\n");
                line_count++;
                col = 0;
            }
            putchar(' ');
            col++;
            i++;
            continue;
        }

        int word_start = i;
        while (i < len && buf[i] != ' ')
            i++;
        int word_len = i - word_start;

        // Перенос слова, если не влезает
        if (col + word_len > WRAP_COL && col > 2) {
            printf("\n");
            line_count++;
            col = 0;
        }

        for (int k = word_start; k < i; k++) {
            putchar(buf[k]);
        }
        col += word_len;
    }

    prev_lines = line_count;
    fflush(stdout);
}

int main(void)
{
    enable_raw_mode();

    char buffer[MAX_LINE];
    int len = 0;

    // Получаем коды Backspace и Ctrl+U из системы
    unsigned char erase = orig_termios.c_cc[VERASE];
    unsigned char kill  = orig_termios.c_cc[VKILL];

    redraw(buffer, len);

    while (1) {
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0)
            break;

        // Ctrl+D (выход)
        if (c == 4) {
            if (len == 0) {
                printf("\r\033[JExiting...\n");
                break;
            }
            continue;
        }

        if (c == erase || c == 127 || c == 8) {        // Backspace (
            if (len > 0)
                len--;
            redraw(buffer, len);
            continue;
        }

        // Ctrl+U (удаление всей строки)
        if (c == kill) {
            len = 0;
            redraw(buffer, len);
            continue;
        }

        // Ctrl+W (удаление последнего слова)
        if (c == 23) {
            while (len > 0 && buffer[len - 1] == ' ')
                len--;
            while (len > 0 && buffer[len - 1] != ' ')
                len--;
            redraw(buffer, len);
            continue;
        }

        // Enter (новая строка)
        if (c == '\n' || c == '\r') {
            printf("\n");
            len = 0;
            prev_lines = 1;
            redraw(buffer, len);
            continue;
        }

        // Печатаемые символы (добавляем в буфер)
        if (isprint(c)) {
            if (len < MAX_LINE - 1) {
                buffer[len++] = (char)c;
                redraw(buffer, len);
            }
            continue;
        }

        // Непечатаемые (звуковой сигнал)
        putchar('\a');
        fflush(stdout);
    }

    return 0;
}
