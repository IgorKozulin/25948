#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

/* Файловый дескриптор нужен обработчику сигнала */
int global_fd = -1;


/* Срабатывает, если прошло 5 секунд */
void alarm_handler(int sig)
{
    (void)sig;

    const char message[] =
        "\n[TIMEOUT] Time is up! Printing full file content...\n";

    write(STDOUT_FILENO, message, sizeof(message) - 1);

    /* Переходим в начало файла */
    lseek(global_fd, 0, SEEK_SET);

    char buffer[256];
    ssize_t bytes_read;

    /* Читаем и выводим весь файл */
    while ((bytes_read = read(global_fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    close(global_fd);

    _exit(0);
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    /* Открываем файл */
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    global_fd = fd;

    LineInfo *table = NULL;

    size_t num_lines = 0;
    size_t capacity = 0;

    off_t line_start = 0;
    size_t line_length = 0;

    char ch;
    ssize_t bytes_read;

    /* Строим таблицу строк */
    while ((bytes_read = read(fd, &ch, 1)) > 0) {

        if (ch == '\n') {

            if (num_lines == capacity) {

                if (capacity == 0)
                    capacity = 10;
                else
                    capacity *= 2;

                LineInfo *temp =
                    realloc(table, capacity * sizeof(LineInfo));

                if (temp == NULL) {
                    printf("Memory allocation error\n");
                    free(table);
                    close(fd);
                    return 1;
                }

                table = temp;
            }

            table[num_lines].offset = line_start;
            table[num_lines].length = line_length;

            num_lines++;

            /*
             * После чтения '\n' текущая позиция
             * является началом следующей строки.
             */
            line_start = lseek(fd, 0L, SEEK_CUR);

            if (line_start == (off_t)-1) {
                perror("lseek");
                free(table);
                close(fd);
                return 1;
            }

            line_length = 0;
        }
        else {
            line_length++;
        }
    }

    if (bytes_read == -1) {
        perror("read");
        free(table);
        close(fd);
        return 1;
    }

    /* Добавляем последнюю строку, если она без '\n' */
    if (line_length > 0) {

        if (num_lines == capacity) {

            if (capacity == 0)
                capacity = 10;
            else
                capacity *= 2;

            LineInfo *temp =
                realloc(table, capacity * sizeof(LineInfo));

            if (temp == NULL) {
                printf("Memory allocation error\n");
                free(table);
                close(fd);
                return 1;
            }

            table = temp;
        }

        table[num_lines].offset = line_start;
        table[num_lines].length = line_length;

        num_lines++;
    }


    /* Печатаем таблицу для проверки */
    printf("\n--- Line Table ---\n");

    for (size_t i = 0; i < num_lines; i++) {
        printf(
            "Line %zu: Offset = %lld, Length = %zu\n",
            i + 1,
            (long long)table[i].offset,
            table[i].length
        );
    }

    printf("------------------\n");


    /* Устанавливаем обработчик SIGALRM */
    signal(SIGALRM, alarm_handler);

    int line_number;

    while (1) {

        printf(
            "\nEnter line number (0 to quit, 5 sec timeout): "
        );

        /*
         * printf может хранить текст в буфере.
         * fflush гарантирует, что приглашение сразу появится.
         */
        fflush(stdout);

        /* Запускаем таймер на 5 секунд */
        alarm(5);

        int result = scanf("%d", &line_number);

        /*
         * Если scanf завершился раньше 5 секунд,
         * таймер больше не нужен.
         */
        alarm(0);

        if (result != 1) {

            printf("Invalid input\n");

            /* Очищаем неправильный ввод */
            int c;

            while ((c = getchar()) != '\n' && c != EOF) {
            }

            continue;
        }

        /* 0 завершает программу */
        if (line_number == 0) {
            break;
        }

        /* Проверяем номер строки */
        if (line_number < 1 ||
            line_number > (int)num_lines) {

            printf("No such line\n");
            continue;
        }

        size_t index = line_number - 1;

        off_t offset = table[index].offset;
        size_t length = table[index].length;

        /* Переходим к нужной строке */
        if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            break;
        }

        char *buffer = malloc(length + 1);

        if (buffer == NULL) {
            printf("Memory allocation error\n");
            break;
        }

        /* Читаем только выбранную строку */
        ssize_t result_read = read(fd, buffer, length);

        if (result_read == -1) {
            perror("read");
            free(buffer);
            break;
        }

        buffer[result_read] = '\0';

        printf("%s\n", buffer);

        free(buffer);
    }

    free(table);
    close(fd);

    return 0;
}
