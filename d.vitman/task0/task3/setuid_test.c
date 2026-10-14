#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

//реальный и эффективный UID 
void print_ids(const char *label) {
    printf("%s\n", label);
    printf("Real UID:      %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
}

// открытие файла и вывод результата
void try_open(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    printf("File '%s' opened successfully\n", filename);
    fclose(fp);
}

int main(void) {
    // UID до изменений
    print_ids("Before setuid");

    try_open("data.txt");

    // сброс привелегий, эффективный UID = реальный 
    if (setuid(getuid()) == -1) {
        perror("setuid");
        return EXIT_FAILURE;
    }

    print_ids("After setuid");

    try_open("data.txt");

    return 0;
}
