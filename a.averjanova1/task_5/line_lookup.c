#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct LineInfo {
    off_t offset;
    int length;
};

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

    struct LineInfo *table = NULL;
    int num_lines = 0;

    off_t line_start = 0;
    int line_length = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            struct LineInfo *temp =
                realloc(table, (num_lines + 1) * sizeof(struct LineInfo));

            if (temp == NULL) {
                free(table);
                close(fd);
                return 1;
            }

            table = temp;

            table[num_lines].offset = line_start;
            table[num_lines].length = line_length;
            num_lines++;

            line_start = lseek(fd, 0L, SEEK_CUR);
            line_length = 0;
        } else {
            line_length++;
        }
    }

    if (line_length > 0) {
        struct LineInfo *temp =
            realloc(table, (num_lines + 1) * sizeof(struct LineInfo));

        if (temp == NULL) {
            free(table);
            close(fd);
            return 1;
        }

        table = temp;

        table[num_lines].offset = line_start;
        table[num_lines].length = line_length;
        num_lines++;
    }

    printf("Line table:\n");

    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: offset = %ld, length = %d\n",
               i + 1,
               (long)table[i].offset,
               table[i].length);
    }

    int number;

    while (1) {
        printf("Enter line number (0 to quit): ");

        if (scanf("%d", &number) != 1) {
            break;
        }

        if (number == 0) {
            break;
        }

        if (number < 1 || number > num_lines) {
            printf("No such line\n");
            continue;
        }

        int index = number - 1;
        int length = table[index].length;

        char *buffer = malloc(length + 1);

        if (buffer == NULL) {
            break;
        }

        lseek(fd, table[index].offset, SEEK_SET);

        if (read(fd, buffer, length) != length) {
            free(buffer);
            break;
        }

        buffer[length] = '\0';

        printf("%s\n", buffer);

        free(buffer);
    }

    free(table);
    close(fd);

    return 0;
}

