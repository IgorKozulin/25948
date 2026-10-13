#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

/* Функция для вывода Real UID и Effective UID */
void print_uids(const char *label) {
    printf("[%s] Real UID = %d, Effective UID = %d\n",
           label, (int)getuid(), (int)geteuid());
}

int main(void) {
    /* 1. Вывод идентификаторов до каких-либо действий */
    print_uids("До открытия файла");

    /* 2. Попытка открыть файл data.txt (права 600, владелец — вы) */
    printf("\nПопытка открыть data.txt (до сброса привилегий):\n");
    FILE *f = fopen("data.txt", "r");
    if (f) {
        printf("  УСПЕХ: файл открыт.\n");
        fclose(f);
    } else {
        perror("  ОШИБКА открытия");
    }

    /* 3. Сброс привилегий: эффективный UID становится равным реальному */
    if (setuid(getuid()) == -1) {
        perror("setuid");
        return EXIT_FAILURE;
    }
    printf("\n--- Привилегии сброшены (setuid(getuid())) ---\n");

    /* 4. Повторный вывод идентификаторов */
    print_uids("После сброса привилегий");

    /* 5. Повторная попытка открыть файл */
    printf("\nПопытка открыть data.txt (после сброса привилегий):\n");
    f = fopen("data.txt", "r");
    if (f) {
        printf("  УСПЕХ: файл открыт.\n");
        fclose(f);
    } else {
        perror("  ОШИБКА открытия");
    }

    return 0;
}
