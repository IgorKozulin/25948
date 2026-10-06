#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

// Функция для вывода текущих идентификаторов
void print_uids(const char* stage) {
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    printf("[%s] Real UID: %d, Effective UID: %d\n", stage, ruid, euid);
}

// Функция для попытки открытия файла
void try_open_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("[-] Ошибка открытия файла");
    } else {
        printf("[+] Файл '%s' успешно открыт!\n", filename);
        fclose(file);
    }
}

int main() {
    const char *filename = "data.txt";

    printf("=== ЭТАП 1: До сброса привилегий ===\n");
    print_uids("Начало работы");
    try_open_file(filename);

    printf("\n=== ЭТАП 2: Вызов setuid(getuid()) ===\n");
    // Сбрасываем эффективный UID до реального
    if (setuid(getuid()) == -1) {
        perror("Ошибка при вызове setuid");
        return EXIT_FAILURE;
    }

    print_uids("После сброса");
    try_open_file(filename);

    return 0;
}
