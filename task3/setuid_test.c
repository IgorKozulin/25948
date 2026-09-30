#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static void print_ids(const char *title)
{
    printf("%s: real UID = %u, effective UID = %u\n",
           title, (unsigned)getuid(), (unsigned)geteuid());
}

static void try_open(const char *title)
{
    FILE *f = fopen("data.txt", "r");

    if (f == NULL) {
        printf("%s: fopen(data.txt) не удалась: ", title);
        perror("");
        return;
    }

    printf("%s: data.txt успешно открыт\n", title);

    char buf[256];
    if (fgets(buf, sizeof(buf), f) != NULL) {
        printf("  первая строка: %s", buf);
    }

    fclose(f);
}

int main(void)
{
    print_ids("До сброса");
    try_open("До сброса");

    if (setuid(getuid()) != 0) {
        perror("setuid(getuid())");
        return EXIT_FAILURE;
    }

    print_ids("После сброса");
    try_open("После сброса");

    return EXIT_SUCCESS;
}
