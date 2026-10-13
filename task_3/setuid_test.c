#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids(const char *stage) {
    printf("=== %s ===\n", stage);
    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("\n");
}

void try_open_file() {
    FILE *f = fopen("data.txt", "r");
    if (f) {
        printf("SUCCESS: File 'data.txt' opened.\n");
        fclose(f);
    } else {
        printf("ERROR: Cannot open 'data.txt'.\n");
        perror("Reason");
    }
    printf("\n");
}

int main() {
    print_uids("Initial state");
    
    printf("Attempt #1 to open data.txt:\n");
    try_open_file();
    
    printf("Calling setuid(getuid()) to drop privileges...\n");
    if (setuid(getuid()) != 0) {
        perror("setuid error");
        return EXIT_FAILURE;
    }
    
    print_uids("After setuid(getuid())");
    
    printf("Attempt #2 to open data.txt:\n");
    try_open_file();
    
    return 0;
}
