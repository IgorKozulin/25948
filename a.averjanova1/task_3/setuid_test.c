#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

void print_ids() {
    printf("Real UID: %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
}

void check_file() {
    FILE *file = fopen("data.txt", "r");

    if (file == NULL) {
        perror("Cannot open data.txt");
    } else {
        printf("data.txt opened successfully\n");
        fclose(file);
    }
}

int main() {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    time_t now;
    time(&now);

    struct tm *sp = localtime(&now);

    FILE *data = fopen("data.txt", "w");

    if (data == NULL) {
        perror("Cannot create data.txt");
        return 1;
    }

    fprintf(data, "%02d/%02d/%04d %02d:%02d %s\n",
            sp->tm_mon + 1,
            sp->tm_mday,
            sp->tm_year + 1900,
            sp->tm_hour,
            sp->tm_min,
            tzname[sp->tm_isdst]);

    fclose(data);

    chmod("data.txt", 0600);

    printf("Before setuid:\n");
    print_ids();
    check_file();

    if (setuid(getuid()) == -1) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    printf("\nAfter setuid:\n");
    print_ids();
    check_file();

    return 0;
}
