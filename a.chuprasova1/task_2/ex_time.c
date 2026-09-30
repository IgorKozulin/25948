#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){
    setenv("TZ","Europe/Moscow", 1);
    tzset();

    time_t now;
    time(&now);

    struct tm *tk;
    tk = localtime(&now);

    printf("%02d/%02d/%04d %02d:%02d:%02d %s\n",
           tk->tm_mday,
           tk->tm_mon + 1,
           tk->tm_year + 1900,
           tk->tm_hour,
           tk->tm_min,
           tk->tm_sec,
           tzname[tk->tm_isdst]);

    return 0;
}

