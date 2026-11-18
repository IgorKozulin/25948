#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("[PARENT] Starting program. My PID: %d\n", getpid());

    pid_t pid1 = fork();
     if (pid1 < 0) {
        perror("fork 1 failed");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        printf("[CHILD 1] I am the first child. My PID: %d\n", getpid());
        
        execlp("cat", "cat", argv[1], NULL);
        
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }
////

    pid_t pid2 = fork();
    
    if (pid2 < 0) {
        perror("fork 2 failed");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        printf("[CHILD 2] I am the second child. My PID: %d\n", getpid());
        printf("[CHILD 2] I am doing some quick work and finishing.\n");
        
        exit(0);
    }

    printf("[PARENT] I am the parent. Child 1 PID: %d, Child 2 PID: %d\n", pid1, pid2);
    printf("[PARENT] Printing some text while children are working...\n");

    int status1, status2;

    printf("[PARENT] Waiting for child 1 to finish...\n");
    waitpid(pid1, &status1, 0);
    
    if (WIFEXITED(status1)) {
        printf("[PARENT] Child 1 exited with status: %d\n", WEXITSTATUS(status1));
    } else {
        printf("[PARENT] Child 1 terminated abnormally\n");
    }

    printf("[PARENT] Waiting for child 2 to finish...\n");
    waitpid(pid2, &status2, 0);
    
    if (WIFEXITED(status2)) {
        printf("[PARENT] Child 2 exited with status: %d\n", WEXITSTATUS(status2));
    } else {
        printf("[PARENT] Child 2 terminated abnormally\n");
    }
    
    printf("[PARENT] All children have finished. This is the last line printed by parent.\n");

    return 0;
} //доба
