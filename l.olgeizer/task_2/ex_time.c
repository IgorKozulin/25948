#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    time_t now;
    struct tm *sp;
    const char *tz = "America/Los_Angeles";
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
            case 't':
                tz = optarg;
                break;
            default:
                fprintf(stderr, "Использование: %s [-t TimeZone] [TimeZone]\n", argv[0]);
                return 1;
        }
    }

    if (optind < argc) {
        tz = argv[optind];
    }

    if (setenv("TZ", tz, 1) != 0) {
        perror("setenv");
        return 1;
    }
    tzset();

    if (time(&now) == (time_t)-1) {
        perror("time");
        return 1;
    }

    sp = localtime(&now);
    if (sp == NULL) {
        perror("localtime");
        return 1;
    }

    printf("%02d/%02d/%04d %02d:%02d %s (%s)\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst],
           tz);

    return 0;
}
