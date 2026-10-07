#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void) {
    setenv("TZ", "America/Los_Angeles", 1); //chasovoi poyas
    tzset();

    time_t now;//зщ�poluchaem vremya
    time(&now);

    struct tm *sp = localtime(&now);//perevodim v lokalynoe

    printf("%02d/%02d/%04d %02d:%02d %s\n",//pechataem
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}
