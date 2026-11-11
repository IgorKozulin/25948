#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pid_t pid;
    char text[256];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid == 0) {
        close(fd[1]);

        ssize_t n;

        while ((n = read(fd[0], text, sizeof(text))) > 0) {
            for (int i = 0; i < n; i++) {
                text[i] = toupper((unsigned char)text[i]);
            }

            if (fwrite(text, 1, n, stdout) != (size_t)n) {
                perror("fwrite");
                break;
            }

            fflush(stdout);
        }

        close(fd[0]);
        return 0;
    }

    close(fd[0]);

    printf("Enter text (empty line to exit):\n");
    fflush(stdout);

    while (fgets(text, sizeof(text), stdin) != NULL) {
        if (text[0] == '\n')
            break;

        int i = 0;
        int len = 0;

        while (text[len] != '\0')
            len++;

        while (i < len) {
            ssize_t n = write(fd[1], text + i, len - i);

            if (n <= 0) {
                perror("write");
                close(fd[1]);
                wait(NULL);
                return 1;
            }

            i += n;
        }
    }

    close(fd[1]);
    wait(NULL);

    return 0;
}
