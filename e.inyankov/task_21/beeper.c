#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

static volatile sig_atomic_t beep_count = 0;

static void sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\a", 1);
    beep_count++;
}

static void sigquit_handler(int sig) {
    (void)sig;
    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), 
                       "\nTotal beeps: %d\nExiting on SIGQUIT.\n", 
                       (int)beep_count);
    if (len > 0) {
        write(STDOUT_FILENO, buffer, (size_t)len);
    }
    _exit(EXIT_SUCCESS);
}

int main(void) {
    struct sigaction sa_int;
    struct sigaction sa_quit;

    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        return EXIT_FAILURE;
    }

    memset(&sa_quit, 0, sizeof(sa_quit));
    sa_quit.sa_handler = sigquit_handler;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction SIGQUIT");
        return EXIT_FAILURE;
    }

    printf("Beeper started.\n");
    printf("Press Ctrl+C for beep (SIGINT).\n");
    printf("Press Ctrl+\\ to exit and print count (SIGQUIT).\n");
    fflush(stdout);

    while (1) {
        pause();
    }

    return EXIT_SUCCESS;
}
