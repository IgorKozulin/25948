#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

void print_uids(const char *stage) {
    printf("--- %s ---\n", stage);
    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
}

void try_write_california_time(const char *filename) {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();
    
    time_t now;
    time(&now);
    
    struct tm *sp = localtime(&now);
    
    char time_str[100];
    snprintf(time_str, sizeof(time_str), "%02d/%02d/%04d %02d:%02d %s\n",
             sp->tm_mday,
             sp->tm_mon + 1,
             sp->tm_year + 1900,
             sp->tm_hour,
             sp->tm_min,
             tzname[sp->tm_isdst]);
    
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "Время в Калифорнии: %s", time_str);
        fclose(file);
        printf("Успех: Время Калифорнии записано в файл '%s'\n", filename);
        printf("Записано: %s", time_str);
    } else {
        printf("Ошибка: Не удалось открыть файл '%s' для записи.\n", filename);
        perror("Причина ошибки");
    }
}

int main() {
    const char *filename = "data.txt";
    
    print_uids("1. Начальное состояние");
    try_write_california_time(filename);
    
    printf("\nВызов setuid(getuid()) для сброса привилегий...\n");
    if (setuid(getuid()) != 0) {
        perror("Ошибка при вызове setuid");
        exit(EXIT_FAILURE);
    }
    
    print_uids("2. После сброса привилегий");
    try_write_california_time(filename);
    
    return 0;
}
