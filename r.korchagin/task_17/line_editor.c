#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>

#define MAX_LINE 40

#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void sig_handler(int sig) {
    (void)sig;
    disable_raw_mode();
    _exit(EXIT_SUCCESS);
}

static void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    atexit(disable_raw_mode);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

static int is_erase_key(char c, cc_t verase) {
    return (c == (char)verase || c == 127 || c == '\b');
}

static int is_kill_key(char c, cc_t vkill) {
    return (c == (char)vkill || c == 21);
}

static int is_werase_key(char c, cc_t vwerase) {
    return (c == 23 || (vwerase != (cc_t)_POSIX_VDISABLE && c == (char)vwerase));
}

static int is_eof_key(char c, cc_t veof) {
    return (c == 4 || (veof != (cc_t)_POSIX_VDISABLE && c == (char)veof));
}

int main(void) {
    enable_raw_mode();

    cc_t verase = orig_termios.c_cc[VERASE];
    cc_t vkill = orig_termios.c_cc[VKILL];
    cc_t veof = orig_termios.c_cc[VEOF];
#ifdef VWERASE
    cc_t vwerase = orig_termios.c_cc[VWERASE];
#else
    cc_t vwerase = (cc_t)_POSIX_VDISABLE;
#endif

    char line[MAX_LINE + 1];
    int len = 0;
    line[0] = '\0';

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (is_eof_key(c, veof)) {
            if (len == 0) {
                printf("\r\nExiting...\r\n");
                fflush(stdout);
                break;
            } else {
                putchar('\a');
                fflush(stdout);
            }
            continue;
        }

        if (c == '\r' || c == '\n') {
            printf("\r\n");
            fflush(stdout);
            len = 0;
            line[0] = '\0';
            continue;
        }

        if (is_erase_key(c, verase)) {
            if (len > 0) {
                printf("\b \b");
                fflush(stdout);
                len--;
                line[len] = '\0';
            } else {
                putchar('\a');
                fflush(stdout);
            }
            continue;
        }

        if (is_kill_key(c, vkill)) {
            if (len > 0) {
                while (len > 0) {
                    printf("\b \b");
                    len--;
                }
                line[0] = '\0';
                fflush(stdout);
            }
            continue;
        }

        if (is_werase_key(c, vwerase)) {
            if (len == 0) {
                putchar('\a');
                fflush(stdout);
            } else {
                int i = len;
                while (i > 0 && line[i - 1] == ' ') {
                    i--;
                }
                while (i > 0 && line[i - 1] != ' ') {
                    i--;
                }
                while (i > 0 && line[i - 1] == ' ') {
                    i--;
                }
                while (len > i) {
                    printf("\b \b");
                    len--;
                }
                line[len] = '\0';
                fflush(stdout);
            }
            continue;
        }

        if (!isprint((unsigned char)c)) {
            putchar('\a');
            fflush(stdout);
            continue;
        }

        if (len < MAX_LINE) {
            line[len++] = c;
            line[len] = '\0';
            putchar(c);
            fflush(stdout);
        } else {
            if (c == ' ') {
                printf("\r\n");
                fflush(stdout);
                len = 0;
                line[0] = '\0';
            } else {
                int word_start = MAX_LINE - 1;
                while (word_start >= 0 && line[word_start] != ' ') {
                    word_start--;
                }
                word_start++;

                int word_len = MAX_LINE - word_start;

                if (word_start == 0 || word_len == 0) {
                    printf("\r\n");
                    putchar(c);
                    fflush(stdout);
                    line[0] = c;
                    len = 1;
                    line[1] = '\0';
                } else {
                    for (int i = 0; i < word_len; i++) {
                        printf("\b \b");
                    }
                    printf("\r\n");

                    char new_line[MAX_LINE + 1];
                    int new_len = 0;
                    for (int i = word_start; i < MAX_LINE; i++) {
                        new_line[new_len++] = line[i];
                        putchar(line[i]);
                    }
                    new_line[new_len++] = c;
                    putchar(c);
                    new_line[new_len] = '\0';

                    memcpy(line, new_line, new_len + 1);
                    len = new_len;
                    fflush(stdout);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
