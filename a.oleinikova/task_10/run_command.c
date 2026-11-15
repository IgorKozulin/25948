#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>   

static void usage(const char *prog);

int main(int argc, char *argv[])
{
    if (argc < 2)                 ъ
        usage(argv[0]);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);   
        perror("execvp");            
        exit(127);                   
    }

    

    int status;
    pid_t w = waitpid(pid, &status, 0);
    if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    if (WIFEXITED(status)) {
        printf("[PARENT] Command exited normally with code: %d\n",
               WEXITSTATUS(status));          
    }
    else if (WIFSIGNALED(status)) {
        printf("[PARENT] Command terminated by signal: %d (%s)\n",
               WTERMSIG(status),
               strsignal(WTERMSIG(status)));  
    }
    else {
        printf("[PARENT] Command terminated abnormally\n");  
    }

    return 0;
}

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <command> [args...]\n", prog);  
    exit(EXIT_FAILURE);
}