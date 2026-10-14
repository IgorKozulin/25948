#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    LineInfo *table = NULL;
    int capacity = 0;
    int num_lines = 0;
    
    long line_start = lseek(fd, 0, SEEK_CUR);
    char ch;

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            long current_pos = lseek(fd, 0, SEEK_CUR);
            
            if (num_lines == capacity) {
                capacity = (capacity == 0) ? 10 : capacity * 2;
                table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
            }
            
            table[num_lines].offset = line_start;
            table[num_lines].length = current_pos - line_start;
            num_lines++;
            
            line_start = current_pos;
        }
    }

    long final_pos = lseek(fd, 0, SEEK_CUR);
    if (final_pos > line_start) {
        if (num_lines == capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            table = (LineInfo *)realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = final_pos - line_start;
        num_lines++;
    }
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", i + 1, table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (line_num == 0) {
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            printf("Неверный номер строки. Доступно строк: %d\n", num_lines);
            continue;
        }

        int idx = line_num - 1;        
        lseek(fd, table[idx].offset, SEEK_SET);
        
        char *buffer = (char *)malloc(table[idx].length + 1);
        read(fd, buffer, table[idx].length);
        buffer[table[idx].length] = '\0';

        printf("%s", buffer);
        free(buffer);
    }
    free(table);
    close(fd);
    return 0;
}
