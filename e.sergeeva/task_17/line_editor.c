#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 256
#define MAX_COL 40

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int main() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) < 0) {
        perror("tcgetattr");
        return 1;
    }

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    atexit(disable_raw_mode);

    char buffer[MAX_LINE];
    int pos = 0;
    int col = 2;
    char c;

    printf("> ");
    fflush(stdout);

    while (1) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        
        if (n <= 0) {
            if (pos == 0) {
                printf("\nExiting...\n");
                break;
            }
            continue;
        }

        if (c == 4) { // Ctrl+D
            if (pos == 0) {
                printf("\nExiting...\n");
                break;
            }
            continue;
        }

        if (c == '\n' || c == '\r') { // Enter
            printf("\n> ");
            pos = 0;
            col = 2;
            memset(buffer, 0, MAX_LINE);
            fflush(stdout);
            continue;
        }

        if (c == orig_termios.c_cc[VERASE] || c == 8 || c == 127) { // Backspace
            if (pos > 0) {
                pos--;
                if (col > 2) {
                    printf("\b \b");
                    col--;
                } else {
                    printf("\n");
                    col = MAX_COL;
                }
                fflush(stdout);
            } else {
                putchar('\a');
                fflush(stdout);
            }
            continue;
        }

        if (c == orig_termios.c_cc[VKILL] || c == 21) { // Ctrl+U
            while (pos > 0) {
                if (col > 2) {
                    printf("\b \b");
                    col--;
                } else {
                    printf("\n");
                    col = MAX_COL;
                }
                pos--;
            }
            fflush(stdout);
            continue;
        }

        if (c == 23) { // Ctrl+W
            while (pos > 0 && isspace((unsigned char)buffer[pos - 1])) {
                pos--;
                if (col > 2) {
                    printf("\b \b");
                    col--;
                } else {
                    printf("\n");
                    col = MAX_COL;
                }
            }
            while (pos > 0 && !isspace((unsigned char)buffer[pos - 1])) {
                pos--;
                if (col > 2) {
                    printf("\b \b");
                    col--;
                } else {
                    printf("\n");
                    col = MAX_COL;
                }
            }
            while (pos > 0 && isspace((unsigned char)buffer[pos - 1])) {
                pos--;
                if (col > 2) {
                    printf("\b \b");
                    col--;
                } else {
                    printf("\n");
                    col = MAX_COL;
                }
            }
            fflush(stdout);
            continue;
        }

        if (isprint((unsigned char)c)) {
            if (pos < MAX_LINE - 1) {
                if (c != ' ' && col + 1 > MAX_COL) {
                    int word_start = pos;
                    while (word_start > 0 && !isspace((unsigned char)buffer[word_start - 1])) {
                        word_start--;
                    }
                    int word_len = pos - word_start;
                    if (word_len > 0 && col > 2) {
                        for (int i = 0; i < word_len; i++) {
                            printf("\b \b");
                        }
                        printf("\n");
                        col = 0;
                        for (int i = word_start; i < pos; i++) {
                            putchar(buffer[i]);
                            col++;
                        }
                    }
                }
                
                buffer[pos] = c;
                pos++;
                
                if (col < MAX_COL) {
                    putchar(c);
                    col++;
                }
                fflush(stdout);
            } else {
                putchar('\a');
                fflush(stdout);
            }
            continue;
        }

        putchar('\a');
        fflush(stdout);
    }

    return 0;
}
