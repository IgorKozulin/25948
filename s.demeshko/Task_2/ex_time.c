#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    // Устанавливаем часовой пояс для Калифорнии
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    time_t now;
    time(&now);

    struct tm *sp = localtime(&now);

    // Формат: MM/DD/YYYY HH:MM TZ
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}
