#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t count = 0;

void on_int(int sig)
{
    count++;
    write(STDOUT_FILENO, "\a", 1);
}

void on_quit(int sig)
{
    char buf[50];
    int n = count;
    int i = 0;
    char digits[20];
    int j = 0;

    do {
        digits[j++] = '0' + n % 10;
        n /= 10;
    } while (n > 0);

    while (j > 0)
        buf[i++] = digits[--j];

    buf[i++] = '\n';

    write(STDOUT_FILENO, "\nBeep count: ", 13);
    write(STDOUT_FILENO, buf, i);

    _exit(0);
}

int main()
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = on_int;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = on_quit;
    sigaction(SIGQUIT, &sa, NULL);

    printf("Press Ctrl+C to beep\n");
    printf("Press Ctrl+\\ to exit\n");
    fflush(stdout);

    while (1)
        pause();

    return 0;
}
