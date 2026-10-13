#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    time_t now;
    struct tm *sp;

    /* 1. Установка часового пояса и инициализация настроек времени */
    if (setenv("TZ", "America/Los_Angeles", 1) != 0) {
        perror("setenv");
        return 1;
    }
    tzset();

    /* 2. Получение текущего календарного времени */
    if (time(&now) == (time_t)-1) {
        perror("time");
        return 1;
    }

    sp = localtime(&now);    /* 3. Преобразование 
    if (sp == NULL) {
        perror("localtime");
        return 1;
    }

    /* 4. Форматированный вывод: месяц (0-11 -> +1), день, год (от 1900 -> +1900), часы, минуты, пояс */
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}
