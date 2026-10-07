#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
void print_uids(const char *stage){
    printf("%s\n", stage);
    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}
void try_open_file(const char *filename){
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        printf("Успех: Файл '%s' успешно открыт!\n", filename);
        fclose(file);
    } else {
        printf("Ошибка: Не удалось открыть файл '%s'.\n", filename);
        perror("Причина ошибки");
    }
}

int main() {
    const char *filename = "data.txt";
    print_uids("1. Начальное состояние");
    try_open_file(filename);
    printf("\nВызов setuid(getuid()) для сброса привилегий...\n");
    if (setuid(getuid()) != 0) {
        perror("Ошибка при вызове setuid");
        exit(EXIT_FAILURE);
    }
    print_uids("2. После сброса привилегий");
    try_open_file(filename);
    return 0;
}
