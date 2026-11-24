#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>

#define MAX_BUFFER 4096
#define LINE_WIDTH 40

struct termios original_term;
char buffer[MAX_BUFFER];
size_t pos = 0;
size_t displayed_lines = 1;

char undo_buffer[MAX_BUFFER];//хранит текст до ластового удаления
size_t undo_pos = 0;//хранит прежнлюю длину
int can_undo = 0;//есть ли удалеие

unsigned char erase_char;
unsigned char kill_char;

//восстановление настроек терминала
void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
}

//настройка терминала
void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &original_term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    erase_char = original_term.c_cc[VERASE];
    kill_char = original_term.c_cc[VKILL];

    if (atexit(disable_raw_mode) != 0) {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    struct termios raw = original_term;

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

//звуковой сигнал
void beep(void) {
    putchar('\a');
    fflush(stdout);
}

//удаление последнего слова
void erase_word(void) {
    while (pos > 0 && buffer[pos - 1] == ' ') {
        pos--;
    }

    while (pos > 0 && buffer[pos - 1] != ' ') {
        pos--;
    }

    buffer[pos] = '\0';
}

//вывод текста с переносом слов
void print_wrapped(void) {
    size_t i = 0;
    size_t col = 0;

    while (i < pos) {
        if (buffer[i] == ' ') {
            if (col == LINE_WIDTH) {
                putchar('\n');
                col = 0;
            }

            putchar(' ');
            col++;
            i++;
            continue;
        }

        size_t start = i;

        while (i < pos && buffer[i] != ' ') {
            i++;
        }

        size_t word_len = i - start;

        if (col > 0 && col + word_len > LINE_WIDTH) {
            putchar('\n');
            col = 0;
        }

        for (size_t j = start; j < i; j++) {
            if (col == LINE_WIDTH) {
                putchar('\n');
                col = 0;
            }

            putchar(buffer[j]);
            col++;
        }
    }

    //сохраняем положение курсора
    if (col == LINE_WIDTH) {
        putchar('\n');
        col = 0;
    }

    (void)col;
}

//полная перерисовка текста
void redraw(void) {
    //возвращаемся к началу отображаемого блока
    printf("\r");

    if (displayed_lines > 1) {
        printf("\033[%zuA", displayed_lines - 1);
    }

    //очищаем старые строки
    for (size_t i = 0; i < displayed_lines; i++) {
        printf("\r\033[2K");

        if (i + 1 < displayed_lines) {
            printf("\033[1B");
        }
    }

    //возвращаемся к первой строке
    if (displayed_lines > 1) {
        printf("\033[%zuA", displayed_lines - 1);
    }

    printf("\r");

    //перерисовываем содержимое буфера
    size_t i = 0;
    size_t col = 0;
    displayed_lines = 1;

    while (i < pos) {
        if (buffer[i] == ' ') {
            if (col == LINE_WIDTH) {
                putchar('\n');
                displayed_lines++;
                col = 0;
            }

            putchar(' ');
            col++;
            i++;
            continue;
        }

        size_t start = i;

        while (i < pos && buffer[i] != ' ') {
            i++;
        }

        size_t word_len = i - start;

        if (col > 0 && col + word_len > LINE_WIDTH) {
            putchar('\n');
            displayed_lines++;
            col = 0;
        }

        for (size_t j = start; j < i; j++) {
            if (col == LINE_WIDTH) {
                putchar('\n');
                displayed_lines++;
                col = 0;
            }

            putchar(buffer[j]);
            col++;
        }
    }

    if (col == LINE_WIDTH) {
        putchar('\n');
        displayed_lines++;
    }

    fflush(stdout);
}

void save_undo(void){
	memcpy(undo_buffer, buffer, pos + 1);
	undo_pos = pos;
	can_undo = 1;
}

void restore_undo(void){
	if (can_undo) {
		memcpy(buffer, undo_buffer, undo_pos + 1);
		pos = undo_pos;
		can_undo = 0;
		redraw();
	}else{
		beep();
	}
}

int main(void) {
    enable_raw_mode();

    buffer[0] = '\0';

    unsigned char c;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        //завершение по ctrl+d
        if (c == 4) {
            if (pos == 0) {
                break;
            }

            beep();
        }

        //удаление последнего символа
        else if (c == erase_char || c == 127 || c == 8) {
            if (pos > 0) {
		save_undo();

                pos--;
                buffer[pos] = '\0';
                redraw();
            } else {
                beep();
            }
        }

        //очистка строки
        else if (c == kill_char) {
            pos = 0;
            buffer[0] = '\0';
            redraw();
        }
	//восстановление последнего удаления ctrl+z
        else if (c == 26) {
            restore_undo();
        }
        //удаление последнего слова
        else if (c == 23) {
            if (pos > 0) {
		save_undo();

                erase_word();
                redraw();
            } else {
                beep();
            }
        }

        //переход на новую строку
        else if (c == '\n' || c == '\r') {
            putchar('\n');
            fflush(stdout);

            pos = 0;
            buffer[0] = '\0';
            displayed_lines = 1;
	    can_undo = 0;
        }

        //обычный печатаемый символ
        else if (isprint(c)) {
            if (pos + 1 < MAX_BUFFER) {
                buffer[pos++] = (char)c;
                buffer[pos] = '\0';
                redraw();
            } else {
                beep();
            }
        }

        //остальные управляющие символы
        else {
            beep();
        }
    }

    putchar('\n');
    fflush(stdout);

    return 0;
}
