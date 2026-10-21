#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s command [arguments]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        perror("execvp");
        exit(127);
    }
    else {
        int status;

        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Command exited with code: %d\n",
                   WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);

            printf("Command terminated by signal: %d (%s)\n",
                   sig, strsignal(sig));
        }
    }

    return 0;
}

