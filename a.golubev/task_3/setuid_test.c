#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids(const char *label)
{
    printf("%s: Real UID = %d, Effective UID = %d\n",
           label, getuid(), geteuid());
}

void try_open(const char *label)
{
    FILE *f = fopen("data.txt", "r");
    if (f == NULL) {
        printf("%s: cannot open data.txt\n", label);
        perror("  reason");
    } else {
        printf("%s: data.txt opened successfully\n", label);
        fclose(f);
    }
}

int main(void)
{
    print_uids("Before privilege drop");
    try_open("Before privilege drop");

    printf("\n--- Calling setuid(getuid()) ---\n\n");

    if (setuid(getuid()) != 0) {
        perror("setuid");
        return EXIT_FAILURE;
    }

    print_uids("After privilege drop");
    try_open("After privilege drop");

    return 0;
}
