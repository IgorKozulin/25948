#include <stdio.h>       
#include <stdlib.h>     
#include <unistd.h>      
#include <fcntl.h>        
#include <signal.h>       
#include <sys/mman.h>     
#include <sys/stat.h>  
#include <string.h>       

//для хранения смещения и длины строки
typedef struct {
    off_t  offset;       
    size_t length;       
} LineInfo;

// глобальные, чтобы обработчик сигнала имел к ним доступ
static char     *g_map       = NULL;   // указатель на отображённую память
static size_t    g_size      = 0;      // размер файла
static LineInfo *g_table     = NULL;   // таблица строк
static int       g_lines_cnt = 0;      // количество строк

// печатает весь файл и выходит
void alarm_handler(int sig)
{
    (void)sig;
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    //отдаём ядру весь отображённый кусок файла
    if (g_map && g_size > 0) {
        write(STDOUT_FILENO, g_map, g_size);
    }
    _exit(EXIT_SUCCESS);   // _exit, а не exit — не трогает stdio
}

// построение таблицы строк
void build_table(void)
{
    //читаем количество строк +1 за каждую \n, плюс последняя, если файл не заканчивается на \n
    int count = 0;
    for (size_t i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') count++;
    }
    if (g_size > 0 && g_map[g_size - 1] != '\n') count++;

    if (count == 0) {
        printf("File is empty.\n");
        return;
    }

    g_table = malloc(sizeof(LineInfo) * count);
    if (!g_table) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    //заполняем таблицу
    size_t line_start = 0;
    int    idx        = 0;
    for (size_t i = 0; i < g_size; i++) {
        if (g_map[i] == '\n') {
            g_table[idx].offset = line_start;
            g_table[idx].length = i - line_start + 1; // включая '\n'
            idx++;
            line_start = i + 1;
        }
    }
    //последняя строка без \n
    if (line_start < g_size) {
        g_table[idx].offset = line_start;
        g_table[idx].length = g_size - line_start;
        idx++;
    }
    g_lines_cnt = idx;

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < g_lines_cnt; i++) {
        printf("Line %d: Offset = %ld, Length = %zu\n",
               i + 1,
               (long)g_table[i].offset,
               g_table[i].length);
    }
    printf("-------------------------\n");
}

// вывод строки по номеру
void print_line(int n)
{
    if (n < 1 || n > g_lines_cnt) {
        printf("Line %d out of range (1..%d).\n", n, g_lines_cnt);
        return;
    }
    LineInfo li = g_table[n - 1];
    // fwrite пишет ровно length байт, не нужен \0 в конце
    fwrite(g_map + li.offset, 1, li.length, stdout);
    //если строка без \n в конце, добавим перевод
    if (li.length > 0 && g_map[li.offset + li.length - 1] != '\n') {
        putchar('\n');
    }
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *filename = argv[1];

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    //размер файла через fstat
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }
    g_size = (size_t)st.st_size;

    // пустой файл — mmap на нулевую длину не сработает 
    if (g_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    // отображаем файл в память
    // PROT_READ    только чтение
    // MAP_PRIVATE  изменения не пойдут в файл 
    // offset=0     с начала файла
    g_map = mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // дескриптор больше не нужен после mmap
    close(fd);

    // строим таблицу строк по отображённой памяти
    build_table();

    signal(SIGALRM, alarm_handler);

    int line_no;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);           

        alarm(5);                 

        if (scanf("%d", &line_no) != 1) {
            alarm(0);              // ввод не число — снимаем таймер
            //очистка буфера
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input.\n");
            continue;
        }

        alarm(0);                 
        if (line_no == 0) break;  
        print_line(line_no);      
    }

    munmap(g_map, g_size);
    free(g_table);
    return EXIT_SUCCESS;
}
