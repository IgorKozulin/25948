#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main(void) {
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe error");
        return EXIT_FAILURE;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork error");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        ssize_t i;

        close(pipefd[1]);

        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            for (i = 0; i < bytes_read; i++) {
                buffer[i] = (char)toupper((unsigned char)buffer[i]);
            }
            write(STDOUT_FILENO, buffer, (size_t)bytes_read);
        }

        close(pipefd[0]);
        _exit(EXIT_SUCCESS);
    } else {
        char input[BUFFER_SIZE];

        close(pipefd[0]);

        printf("[PARENT] Enter text (empty line or Ctrl+D to exit):\n");
        fflush(stdout);

        while (fgets(input, sizeof(input), stdin) != NULL) {
            if (input[0] == '\n' || input[0] == '\r') {
                break;
            }
            write(pipefd[1], input, strlen(input));
        }

        close(pipefd[1]);
        wait(NULL);

        printf("[PARENT] Child finished. Parent exiting.\n");
    }

    return EXIT_SUCCESS;
}
