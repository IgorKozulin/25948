#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    size_t offset;
    size_t length;
} LineInfo;

/* Отображённый в память файл */
char *global_map = NULL;

/* Размер файла */
size_t global_size = 0;


/*
 * Эта функция вызывается,
 * если пользователь не ввёл число за 5 секунд.
 */
void alarm_handler(int sig)
{
    (void)sig;

    /*
     * Используем fwrite вместо write.
     * Данные берём непосредственно из mmap.
     */
    printf("\n[TIMEOUT] Time is up! Printing full file content...\n");

    fwrite(global_map, 1, global_size, stdout);

    /*
     * Сбрасываем буфер stdout, чтобы текст
     * гарантированно появился перед завершением.
     */
    fflush(stdout);

    _exit(0);
}


int main(int argc, char *argv[])
{
    /* Проверяем наличие имени файла */
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


    /*
     * Получаем информацию о файле.
     * В частности, нам нужен его размер.
     */
    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    global_size = st.st_size;


    /* mmap нельзя нормально использовать для пустого файла */
    if (global_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }


    /*
     * Отображаем весь файл в память.
     *
     * PROT_READ   - только чтение
     * MAP_PRIVATE - приватное отображение
     */
    global_map = mmap(
        NULL,
        global_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (global_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }


    /*
     * После успешного mmap файловый дескриптор
     * нам больше не нужен.
     */
    close(fd);


    /* Таблица строк */
    LineInfo *table = NULL;

    size_t num_lines = 0;
    size_t capacity = 0;

    size_t line_start = 0;


    /*
     * Проходим по отображённому файлу
     * как по обычному массиву символов.
     */
    for (size_t i = 0; i < global_size; i++) {

        if (global_map[i] == '\n') {

            /* Если таблица заполнена, увеличиваем её */
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
                    munmap(global_map, global_size);

                    return 1;
                }

                table = temp;
            }


            /*
             * Начало строки.
             */
            table[num_lines].offset = line_start;

            /*
             * i указывает на '\n'.
             * Поэтому длина:
             *
             * i - line_start
             */
            table[num_lines].length = i - line_start;

            num_lines++;


            /*
             * Следующая строка начинается
             * сразу после '\n'.
             */
            line_start = i + 1;
        }
    }


    /*
     * Если последняя строка файла
     * не заканчивается символом '\n'.
     */
    if (line_start < global_size) {

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
                munmap(global_map, global_size);

                return 1;
            }

            table = temp;
        }


        table[num_lines].offset = line_start;

        table[num_lines].length =
            global_size - line_start;

        num_lines++;
    }


    /* Выводим таблицу для проверки */
    printf("\n--- Line Table ---\n");

    for (size_t i = 0; i < num_lines; i++) {

        printf(
            "Line %zu: Offset = %zu, Length = %zu\n",
            i + 1,
            table[i].offset,
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

        fflush(stdout);


        /* Даём пользователю 5 секунд */
        alarm(5);


        /* Ждём номер строки */
        int result = scanf("%d", &line_number);


        /* Пользователь успел - отменяем таймер */
        alarm(0);


        /* Проверяем ввод */
        if (result != 1) {

            printf("Invalid input\n");

            int c;

            while ((c = getchar()) != '\n' && c != EOF) {
            }

            continue;
        }


        /* 0 - завершение программы */
        if (line_number == 0) {
            break;
        }


        /* Проверяем существование строки */
        if (
            line_number < 1 ||
            line_number > (int)num_lines
        ) {
            printf("No such line\n");
            continue;
        }


        /*
         * Пользователь считает строки с 1,
         * массив начинается с 0.
         */
        size_t index = line_number - 1;

        size_t offset = table[index].offset;
        size_t length = table[index].length;


        /*
         * Самая важная часть задачи 7.
         *
         * Никакого lseek и read.
         *
         * global_map + offset
         * сразу указывает на начало нужной строки.
         */
        fwrite(
            global_map + offset,
            1,
            length,
            stdout
        );

        printf("\n");
    }


    /* Освобождаем таблицу */
    free(table);


    /* Удаляем отображение файла из памяти */
    if (munmap(global_map, global_size) == -1) {
        perror("munmap");
        return 1;
    }

    return 0;
}
