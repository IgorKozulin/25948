#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

int main() {
    FILE *file;
    time_t now;
    struct tm *sp;

    // Выводим UID до смены прав
    printf("Do setuid: Real UID = %d, Effective UID = %d\n", getuid(), geteuid());

    // Устанавливаем калифорнийское время
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    // Получаем текущее время
    time(&now);
    sp = localtime(&now);

    // Выводим калифорнийское время
    printf("Vremya v Kalifornii: %02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
           sp->tm_hour, sp->tm_min, tzname[sp->tm_isdst]);

    // Пробуем открыть файл в первый раз
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Oshibka otkrytiya 1");
    } else {
        printf("Fayl otkryt uspeshno (1)\n");
        fclose(file);
    }

    // Сбрасываем привилегии
    printf("Vyzov setuid(getuid()) dlya sbrosa privilegiy...\n");
    setuid(getuid());

    // Выводим UID после смены прав
    printf("Posle setuid: Real UID = %d, Effective UID = %d\n", getuid(), geteuid());

    // Пробуем открыть файл во второй раз
    file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Oshibka otkrytiya 2");
    } else {
        printf("Fayl otkryt uspeshno (2)\n");
        fclose(file);
    }

    return 0;
}
