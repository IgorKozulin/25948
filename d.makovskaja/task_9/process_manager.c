#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        printf("[CHILD] I am the child process. My PID: %d\n", getpid());
        
        execlp("cat", "cat", argv[1], NULL);
        
        perror("[CHILD] execlp failed");
        exit(EXIT_FAILURE);
    } else {
        printf("[PARENT] Starting program. My PID: %d\n", getpid());
        printf("[PARENT] I am the parent. Child PID: %d\n", pid);
        printf("[PARENT] Printing some text while child is working...\n");
        
        printf("[PARENT] Waiting for child to finish...\n");
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("[PARENT] Child exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("[PARENT] Child did not exit normally.\n");
        }
        
        printf("[PARENT] Child has finished. This is the last line printed by parent.\n");
    }

    return 0;
}
