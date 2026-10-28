#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

struct termios old_term;

void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term);
}

void enable_raw_mode()
{
    struct termios new_term;

    tcgetattr(STDIN_FILENO, &old_term);

    new_term = old_term;

    new_term.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    new_term.c_cc[VMIN] = 1;
    new_term.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_term);

    atexit(disable_raw_mode);
}

void redraw(char *buffer, int pos)
{
    printf("\r\033[K");

    int col = 0;
    int i = 0;

    while (i < pos) {
        if (buffer[i] == ' ') {
            putchar(' ');
            col++;
            i++;
            continue;
        }

        int start = i;

        while (i < pos && buffer[i] != ' ') {
            i++;
        }

        int word_length = i - start;

        if (col > 0 && col + word_length > 40) {
            printf("\n");
            col = 0;
        }

        for (int j = start; j < i; j++) {
            putchar(buffer[j]);
            col++;
        }
    }

    fflush(stdout);
}

int main()
{
    char buffer[4096];
    int pos = 0;
    char c;

    enable_raw_mode();

    char erase_char = old_term.c_cc[VERASE];
    char kill_char = old_term.c_cc[VKILL];

    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) {
            break;
        }

        if (c == 4) {
            if (pos == 0) {
                printf("\r\nExiting...\r\n");
                break;
            }
        }
        else if (c == erase_char) {
            if (pos > 0) {
                pos--;
                redraw(buffer, pos);
            }
        }
        else if (c == kill_char) {
            pos = 0;
            redraw(buffer, pos);
        }
        else if (c == 23) {
            while (pos > 0 && buffer[pos - 1] == ' ') {
                pos--;
            }

            while (pos > 0 && buffer[pos - 1] != ' ') {
                pos--;
            }

            while (pos > 0 && buffer[pos - 1] == ' ') {
                pos--;
            }

            redraw(buffer, pos);
        }
        else if (c == '\n' || c == '\r') {
            printf("\r\n");
            pos = 0;
        }
        else if (isprint((unsigned char)c)) {
            if (pos < 4095) {
                buffer[pos] = c;
                pos++;
                redraw(buffer, pos);
            }
        }
        else {
            putchar('\a');
            fflush(stdout);
        }
    }

    return 0;
}
