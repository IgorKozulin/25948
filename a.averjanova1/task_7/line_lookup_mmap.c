#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

char *file_map;
size_t file_size;

void alarm_handler(int sig) {
    (void)sig;

    write(STDOUT_FILENO, "\nTime is up!\n", 13);
    write(STDOUT_FILENO, file_map, file_size);

    _exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    file_size = st.st_size;

    if (file_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (file_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    struct LineInfo *table = NULL;
    int num_lines = 0;

    size_t line_start = 0;

    for (size_t i = 0; i < file_size; i++) {
        if (file_map[i] == '\n') {
            struct LineInfo *temp =
                realloc(table, (num_lines + 1) * sizeof(struct LineInfo));

            if (temp == NULL) {
                free(table);
                munmap(file_map, file_size);
                return 1;
            }

            table = temp;

            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start;

            num_lines++;
            line_start = i + 1;
        }
    }

    if (line_start < file_size) {
        struct LineInfo *temp =
            realloc(table, (num_lines + 1) * sizeof(struct LineInfo));

        if (temp == NULL) {
            free(table);
            munmap(file_map, file_size);
            return 1;
        }

        table = temp;

        table[num_lines].offset = line_start;
        table[num_lines].length = file_size - line_start;
        num_lines++;
    }

    printf("Line table:\n");

    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: offset = %ld, length = %zu\n",
               i + 1,
               (long)table[i].offset,
               table[i].length);
    }

    signal(SIGALRM, alarm_handler);

    int number;

    while (1) {
        printf("Enter line number (0 to quit, 5 sec): ");
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &number) != 1) {
            alarm(0);
            break;
        }

        alarm(0);

        if (number == 0) {
            break;
        }

        if (number < 1 || number > num_lines) {
            printf("No such line\n");
            continue;
        }

        int index = number - 1;

        fwrite(file_map + table[index].offset,
               1,
               table[index].length,
               stdout);

        printf("\n");
    }

    free(table);
    munmap(file_map, file_size);

    return 0;
}

