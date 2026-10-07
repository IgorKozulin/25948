#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids(const char *stage) {
    printf("[%s] Real UID: %d, Effective UID: %d\n", stage, (int)getuid(), (int)geteuid());
}

void try_open_file(const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("  fopen error");
    } else {
        printf("  Файл успешно открыт!\n");
        fclose(fp);
    }
}

int main(void) {
    const char *filepath = "data.txt";

    // 1. Проверка до сброса привилегий
    print_uids("До сброса привилегий");
    try_open_file(filepath);

    // 2. Сброс привилегий (euid становится равен ruid)
    if (setuid(getuid()) != 0) {
        perror("Ошибка вызова setuid");
        return EXIT_FAILURE;
    }

    // 3. Повторная проверка после сброса
    print_uids("После сброса привилегий");
    try_open_file(filepath);

    return EXIT_SUCCESS;
}
