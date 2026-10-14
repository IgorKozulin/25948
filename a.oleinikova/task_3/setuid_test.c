#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static void show_ids(const char *label);
static void f_open(const char *path);


int main(void) {
    show_ids("before");
    f_open("data.txt");

    if (setuid(getuid()) != 0) {
         perror("setuid");
    }

    show_ids("after");
    f_open("data.txt");
    return 0;
}

static void show_ids(const char *label) {
    printf("[%s] Real UID=%d, Effective UID=%d\n",label, getuid(), geteuid());
}

static void f_open(const char *path) {
    FILE *f = fopen(path, "r");
    if (f){
        printf("Opened %s successfully\n", path); 
        fclose(f); 
    }
    else{
        perror("fopen");
    }
}

