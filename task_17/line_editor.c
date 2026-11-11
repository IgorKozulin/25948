#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LEN 1024

/* Сохраняем исходные настройки терминала, чтобы вернуть их при выходе */
struct termios orig_termios;

/* Функция восстановления терминала */
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/* Функция включения raw mode */
void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios); // запоминаем как было
    raw = orig_termios;
    
    // Отключаем эхо (ECHO), канонический режим (ICANON) и расширенную обработку (IEXTEN)
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    
    // Читаем по 1 байту
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    // Регистрируем функцию, которая выполнится при выходе (даже если Ctrl+C)
    atexit(disable_raw_mode);
}

/* Функция перерисовки экрана с учетом переноса слов */
void redraw(const char *buffer, int pos) {
    // Возвращаемся в начало строки и очищаем её до конца
    printf("\r\033[K");
    
    int col = 0;
    int i = 0;
    
    while (i < pos) {
        // Если текущий символ — пробел или конец слова
        if (buffer[i] == ' ') {
            // Просто печатаем пробел, если есть место
            if (col >= 40) {
                printf("\n\033[K");
                col = 0;
            }
            putchar(' ');
            col++;
            i++;
        } else {
            // Мы в начале слова, ищем его конец
            int word_start = i;
            while (i < pos && buffer[i] != ' ') {
                i++;
            }
            int word_len = i - word_start;
            
            // Если слово не влезает в текущую строку (до 40 символов), переносим его
            if (col + word_len > 40) {
                printf("\n\033[K");
                col = 0;
            }
            
            // Печатаем слово
            for (int j = word_start; j < i; j++) {
                putchar(buffer[j]);
            }
            col += word_len;
        }
    }
    
    // Стираем остаток старой строки, если новая короче
    printf("\033[K");
    fflush(stdout);
}

int main() {
    enable_raw_mode();
    
    char buffer[MAX_LEN];
    int pos = 0;
    memset(buffer, 0, sizeof(buffer));
    
    // Получаем системные символы ERASE (Backspace) и KILL (Ctrl+U)
    unsigned char erase_char = orig_termios.c_cc[VERASE];
    unsigned char kill_char = orig_termios.c_cc[VKILL];

    printf("Line editor started. Ctrl+D to exit.\r\n> ");
    fflush(stdout);

    char c;
    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        // 1. Выход: Ctrl+D (код 4) на пустой строке
        if (c == 4) { // Ctrl+D
            if (pos == 0) {
                printf("\r\nExiting...\r\n");
                break;
            }
        }
        // 2. ERASE (Backspace): стираем последний символ
        else if (c == erase_char || c == 127 || c == 8) {
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                redraw(buffer, pos);
            }
        }
        // 3. KILL (Ctrl+U): стираем всю строку
        else if (c == kill_char || c == 21) {
            pos = 0;
            buffer[0] = '\0';
            redraw(buffer, pos);
        }
        // 4. Ctrl+W (код 23): стираем последнее слово
        else if (c == 23) {
            // Убираем пробелы в конце
            while (pos > 0 && isspace(buffer[pos - 1])) {
                pos--;
            }
            // Убираем само слово
            while (pos > 0 && !isspace(buffer[pos - 1])) {
                pos--;
            }
            buffer[pos] = '\0';
            redraw(buffer, pos);
        }
        // 5. Enter: переходим на новую строку
        else if (c == '\n' || c == '\r') {
            printf("\r\n> ");
            pos = 0;
            buffer[0] = '\0';
            fflush(stdout);
        }
        // 6. Печатаемые символы
        else if (isprint((unsigned char)c)) {
            if (pos < MAX_LEN - 1) {
                buffer[pos++] = c;
                buffer[pos] = '\0';
                redraw(buffer, pos);
            }
        }
        // 7. Всё остальное — звуковой сигнал
        else {
            printf("\a");
            fflush(stdout);
        }
    }

    return 0;
}
