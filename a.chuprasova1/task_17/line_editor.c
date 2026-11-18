
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>

#define MAX_LEN 4095
#define WIDTH 40

struct termios original;
int displayed_rows = 0; //переход строк

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original); // вернуть настройки терм
}

void enable_raw_mode(void) { //настройка терминала
    if (tcgetattr(STDIN_FILENO, &original) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    struct termios raw = original;

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 1;  //1 байт
    raw.c_cc[VTIME] = 0; //откл таймер

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }

    atexit(disable_raw_mode); //регистр по завершении
}

void redraw(const char *buffer, int pos) { //отображение
    printf("\r"); //курсор в начало

    for (int i = 0; i < displayed_rows; i++) {
        printf("\033[A");// на с троку вверх
    }

    printf("\r\033[J"); //отчисть от курсора

    int col = 0;
    displayed_rows = 0;

    for (int i = 0; i < pos;) { //обработка буфера
        if (buffer[i] == ' ') {
            if (col == WIDTH) {
                putchar('\n');
                displayed_rows++;
                col = 0;
            }

            putchar(' ');
            col++;
            i++;
        } else {
            int start = i;

            while (i < pos && buffer[i] != ' ') { //разбив на слова
                i++;
            }

            int len = i - start;

            if (col > 0 && col + len > WIDTH) { //перенос
                putchar('\n');
                displayed_rows++;
                col = 0;
            }

            for (int j = start; j < i; j++) { //вывод
                if (col == WIDTH) {
                    putchar('\n');
                    displayed_rows++;
                    col = 0;
                }

                putchar(buffer[j]);
                col++;
            }
        }
    }

    fflush(stdout);
}

int main(void) {
    enable_raw_mode();

    char buffer[MAX_LEN + 1];
    int pos = 0;
    char c;

    char undo_buffer[MAX_LEN + 1];
    int undo_pos = 0;
    int can_undo = 0;

    while (read(STDIN_FILENO, &c, 1) == 1) {

        if (c == 4) { //ctrl+d
            if (pos == 0) {
                break;
            }
            putchar('\a');
        }

        else if (c == '\n' || c == '\r') { //enter
            putchar('\n');
            fflush(stdout);
            pos = 0;
            displayed_rows = 0;

            can_undo = 0;
        }

        else if ((unsigned char)c ==
                 (unsigned char)original.c_cc[VERASE]) {  //backspace
            if (pos > 0) {

                memcpy(undo_buffer, buffer, pos);
                undo_pos = pos;
                can_undo = 1;

                pos--;
                redraw(buffer, pos);
            }
        }

        else if ((unsigned char)c ==
                 (unsigned char)original.c_cc[VKILL]) { //ctrl+u очистка строки

            if (pos > 0) {
                memcpy(undo_buffer, buffer, pos);
                undo_pos = pos;
                can_undo = 1;
            }


            pos = 0;
            redraw(buffer, pos);
        }

        else if (c == 23) { //ctrl+w  удаление слова

            if (pos > 0) {
                memcpy(undo_buffer, buffer, pos);
                undo_pos = pos;
                can_undo = 1;
            }

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

        else if (c == 26) { //ctrl+z
            if (can_undo) {
                memcpy(buffer, undo_buffer, undo_pos);
                pos = undo_pos;
                can_undo = 0;
                redraw(buffer, pos);
            } else {
                putchar('\a');
            }
        }

                                                // обычн символы
        else if (isprint((unsigned char)c)) {
            if (pos < MAX_LEN) {

                can_undo = 0;

                buffer[pos++] = c;
                redraw(buffer, pos);
            } else {
                putchar('\a');
            }
        }

        else {
            putchar('\a');
        }

        fflush(stdout);
    }

    printf("\nExiting...\n");

    return 0;
}

