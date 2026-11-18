#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static struct termios orig_termios;

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
    char line[128];
    int len = 0;
    char c;
    cc_t erase_char;
    cc_t kill_char;

    enable_raw_mode();

    erase_char = orig_termios.c_cc[VERASE];
    kill_char = orig_termios.c_cc[VKILL];

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 4) {
            if (len == 0) {
                write(STDOUT_FILENO, "\r\n", 2);
                break;
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == erase_char || c == 127 || c == 8) {
            if (len > 0) {
                len--;
                line[len] = '\0';
                write(STDOUT_FILENO, "\b \b", 3);
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == kill_char || c == 21) {
            if (len > 0) {
                while (len > 0) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                }
                line[0] = '\0';
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == 23) {
            if (len > 0) {
                while (len > 0 && line[len - 1] == ' ') {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                }
                while (len > 0 && line[len - 1] != ' ') {
                    write(STDOUT_FILENO, "\b \b", 3);
                    len--;
                }
                line[len] = '\0';
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == '\n' || c == '\r') {
            write(STDOUT_FILENO, "\r\n", 2);
            len = 0;
            line[0] = '\0';
        }
        else if (isprint((unsigned char)c)) {
            if (len < 40) {
                line[len++] = c;
                line[len] = '\0';
                write(STDOUT_FILENO, &c, 1);
            } else {
                if (c == ' ') {
                    write(STDOUT_FILENO, "\r\n", 2);
                    len = 0;
                    line[0] = '\0';
                } else {
                    int w_start = len - 1;
                    while (w_start >= 0 && line[w_start] != ' ') {
                        w_start--;
                    }
                    w_start++;

                    if (w_start == 0) {
                        write(STDOUT_FILENO, "\r\n", 2);
                        line[0] = c;
                        len = 1;
                        line[1] = '\0';
                        write(STDOUT_FILENO, &c, 1);
                    } else {
                        int w_len = len - w_start;
                        int i;
                        for (i = 0; i < w_len; i++) {
                            write(STDOUT_FILENO, "\b \b", 3);
                        }
                        write(STDOUT_FILENO, "\r\n", 2);

                        char new_word[128];
                        memcpy(new_word, &line[w_start], w_len);
                        new_word[w_len] = c;
                        w_len++;
                        new_word[w_len] = '\0';

                        write(STDOUT_FILENO, new_word, w_len);
                        memcpy(line, new_word, w_len + 1);
                        len = w_len;
                    }
                }
            }
        }
        else {
            write(STDOUT_FILENO, "\a", 1);
        }
    }

    return 0;
}

