#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static void print_uids(const char *label)
{
    printf("--- %s ---\n", label);
    printf("Real UID      (getuid)  = %d\n", (int)getuid());
    printf("Effective UID (geteuid) = %d\n", (int)geteuid());
    printf("\n");
}

static void try_open(const char *label)
{
    FILE *f = fopen("data.txt", "r");

    printf("--- %s ---\n", label);
    if (f == NULL) {
        perror("fopen(data.txt)");
    } else {
        printf("fopen(data.txt): OK\n");

        char buf[256];
        if (fgets(buf, sizeof(buf), f) != NULL) {
            printf("First line: %s", buf);
        }
        fclose(f);
    }
    printf("\n");
}

int main(void)
{
    print_uids("Before dropping privileges");

    try_open("Open data.txt BEFORE setuid(getuid())");

    if (setuid(getuid()) != 0) {
        perror("setuid(getuid())");
        exit(EXIT_FAILURE);
    }
    printf(">>> Privileges dropped via setuid(getuid())\n\n");


    print_uids("After dropping privileges");

    try_open("Open data.txt AFTER setuid(getuid())");

    return 0;
}
