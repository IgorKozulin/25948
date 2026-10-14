#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

char *global_map = NULL;
long global_size = 0;

void alarm_handler(int sig) {
    const char *msg = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, global_map, global_size);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Ошибка открытия файла");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Ошибка получения размера файла");
        close(fd);
        return 1;
    }

    global_size = st.st_size;
    if (global_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    global_map = (char *)mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED) {
        perror("Ошибка mmap");
        close(fd);
        return 1;
    }

    close(fd);

    LineInfo *table = NULL;
    int capacity = 0;
    int num_lines = 0;
    long line_start = 0;

    for (long i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines == capacity) {
                capacity = (capacity == 0) ? 10 : capacity * 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start + 1;
            num_lines++;
            line_start = i + 1;
        }
    }

    if (line_start < global_size) {
        if (num_lines == capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = global_size - line_start;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    int line_num;
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &line_num) != 1) {
            alarm(0);
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Неверный ввод. Пожалуйста, введите число.\n");
            continue;
        }

        alarm(0);

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Неверный номер строки. Доступно строк: %d\n", num_lines);
            continue;
        }

        int idx = line_num - 1;
        fwrite(global_map + table[idx].offset, 1, table[idx].length, stdout);
    }

    free(table);
    munmap(global_map, global_size);
    return 0;
}
