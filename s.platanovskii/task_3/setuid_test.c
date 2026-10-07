#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static void print_ids(const char *label) { //pechat uid
    printf("[%s] Real UID = %d, Effective UID = %d\n",
           label, (int)getuid(), (int)geteuid());
}

static void try_open(const char *label) { //otkrivaem file
    FILE *f = fopen("data.txt", "r");
    if (f == NULL) {
        perror(label);
    } else {
        printf("[%s] data.txt opened successfully\n", label);
        fclose(f);
    }
}

int main(void) {
    print_ids("before");
    try_open("before");

    if (setuid(getuid()) != 0)
        perror("setuid");

    print_ids("after");
    try_open("after");

    return 0;
}
