#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>



static void usage(const char *prog);

int main(int argc, char *argv[]){
    
    if (argc != 2)
        usage(argv[0]);

    const char *name_f = argv[1];

    
    pid_t pid = fork();// 0 - ребенок, 1 - родитель, -1 - ошибка
    if (pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        printf("[CHILD] I am the child process. My PID: %d\n", getpid());
        fflush(stdout);  
        execlp("cat", "cat", name_f, (char *)NULL);// подмена программы
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    
    printf("[PARENT] Starting program. My PID: %d\n", getpid());
    printf("[PARENT] I am the parent. Child PID: %d\n", pid);
    printf("[PARENT] Printing some text while child is working...\n");

  
    printf("[PARENT] Waiting for child to finish...\n");
    fflush(stdout);

    int status;
    pid_t w = waitpid(pid, &status, 0);
    if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    
    if (WIFEXITED(status)){// нормальное завершенеи
        printf("[PARENT] Child exited with status: %d\n", WEXITSTATUS(status)); //выводит ретерн
    } 

    else if (WIFSIGNALED(status)) {//принудиткльно убит
        printf("[PARENT] Child killed by signal: %d\n",WTERMSIG(status)); //каким сигналом
    } 
    
    else {
        printf("[PARENT] Child terminated abnormally\n");
    }
    printf("[PARENT] Child has finished. This is the last line printed by parent.\n");


    printf("[PARENT] I finish\n");
    return 0;
}



static void usage(const char *prog){
    fprintf(stderr, "Usage: %s <file>\n", prog);
    exit(EXIT_FAILURE);
}

