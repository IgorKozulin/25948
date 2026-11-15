#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LINE 40     
#define BUF_SIZE 4096

static struct termios orig; // исходные настройки терминала


static void raw_mode(void);
static void not_raw_mode(void);
static void redraw(const char *buf, int pos);
static void  del_one(char *buf, int *pos);
static void clear(char *buf, int *pos);
static void del_last_w(char *buf, int *pos);


int main(void){
    raw_mode();//переключить терминал в raw mode

    char buffer[BUF_SIZE];
    int pos = 0;
    char c;

    while (read(STDIN_FILENO, &c, 1) == 1){//прочитать ровно один байт



        if (c == 4 && pos == 0){// Ctrl+D и курсор в начале строки
            break;
        } 
        else if (c == orig.c_cc[VERASE]){//удалить символ
            del_one(buffer, &pos);
        }
        else if (c == orig.c_cc[VKILL]){//очистить строку
            clear(buffer, &pos);
        }
        else if (c == 23){
            del_last_w(buffer, &pos);   /* Ctrl+W */
        }
        else if (c == '\n' || c == '\r'){
            putchar('\n');
            pos = 0;
        }
        else if (isprint((unsigned char)c)) {
            if (pos < MAX_LINE){
                buffer[pos++] = c;
            }
        }
        else {
            putchar('\a'); // звуковой сигнал
        }
        redraw(buffer, pos);
    }

    printf("\nExiting...\n");
    return 0;
}







static void not_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

static void raw_mode(void)
{
    if (tcgetattr(STDIN_FILENO, &orig) == -1){
        perror("tcgetattr"); 
        exit(1);}
    struct termios raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);  
    raw.c_cc[VMIN]  = 1;                        
    raw.c_cc[VTIME] = 0;                        

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
        perror("tcsetattr"); 
        exit(1);}

    atexit(not_raw_mode);
}


static void del_one(char *buf, int *pos)
{
    if (*pos > 0) (*pos)--;
}

static void handle_kill(char *buf, int *pos)
{
    *pos = 0;
}

static void handle_ctrl_w(char *buf, int *pos)
{
    while (*pos > 0 && isspace((unsigned char)buf[*pos - 1])){
        // пропустить пробелы до слова
        (*pos)--;
    }
    
    while (*pos > 0 && !isspace((unsigned char)buf[*pos - 1])){
        // пропустить буквы до конца слова
        (*pos)--;
    }
}


static void redraw(const char *buf, int pos){
    printf("\r\033[K");   //очистить строку от курcора и до конца
    int how_many_now_s = 0;
    int i = 0;
    while (i < pos) {
        int start = i;
        while (i < pos && !isspace((unsigned char)buf[i])){
            i++;
        }
        int word_len = i - start;

        if (how_many_now_s > 0 && how_many_now_s + word_len > MAX_LINE){
            printf("\n\033[K");
            how_many_now_s = 0;
        }

        
        for (int k = start; k < i; k++){
            putchar(buf[k]);
        }

        how_many_now_s += word_len;

        while (i < pos && isspace((unsigned char)buf[i])) {
            putchar(buf[i]);
            how_many_now_s++;
            i++;
        }
    }

    fflush(stdout);
}