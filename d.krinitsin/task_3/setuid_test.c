#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define DATA_FILE "data.txt"

static void print_ids(void)
{
    printf("Real UID      = %d\n", getuid());
    printf("Effective UID = %d\n", geteuid());
}

static void try_open(const char *label)
{
    FILE *f = fopen(DATA_FILE, "r");
    if (f == NULL) {
        printf("%s: cannot open %s: ", label, DATA_FILE);
        perror("");
    } else {
        printf("%s: opened %s successfully\n", label, DATA_FILE);
        fclose(f);
    }
}

int main(void)
{
    printf("=== Before setuid ===\n");
    print_ids();
    try_open("First try");

    printf("\n=== Calling setuid(getuid()) ===\n");
    if (setuid(getuid()) == -1) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    printf("\n=== After setuid ===\n");
    print_ids();
    try_open("Second try");

    return 0;
}
