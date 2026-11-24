/* Задача 21: пищалка — SIGINT пищит, SIGQUIT завершает с подсчётом */
#define _DEFAULT_SOURCE          /* нужен ECHOCTL */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

static volatile sig_atomic_t beep_count = 0;   /* считает SIGINT */
static volatile sig_atomic_t quit_flag  = 0;   /* выставит SIGQUIT */

/* ---------- отключение эха ^C (необязательно, но с ним «тихо») ---------- */
#ifdef ECHOCTL
static struct termios saved_tio;
static int tio_saved = 0;

static void echoctl_off(void)
{
    struct termios t;
    if (tcgetattr(STDIN_FILENO, &t) == -1)
        return;
    saved_tio = t;
    t.c_lflag &= ~(tcflag_t)ECHOCTL;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &t) == -1)
        return;
    tio_saved = 1;
}

static void echoctl_restore(void)
{
    if (tio_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_tio);
        tio_saved = 0;
    }
}
#else
static void echoctl_off(void)     { }
static void echoctl_restore(void) { }
#endif

/* ---------- обработчики: только async-signal-safe действия ---------- */
static void on_sigint(int sig)
{
    ssize_t rc;

    (void)sig;
    beep_count++;                                   /* volatile sig_atomic_t */
    rc = write(STDOUT_FILENO, "\a", 1);             /* BEL — безопасно */
    (void)rc;
}

static void on_sigquit(int sig)
{
    (void)sig;
    quit_flag = 1;          /* печать и восстановление терминала — в main */
}

static void install_handler(int signo, void (*fn)(int), int flags)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = fn;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = flags;

    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    sigset_t block, waitmask;

    install_handler(SIGINT,  on_sigint,  SA_RESTART);
    install_handler(SIGQUIT, on_sigquit, 0);

    echoctl_off();

    printf("Пищалка запущена!\n");
    printf("Нажмите Ctrl+C — будет звуковой сигнал\n");
    printf("Нажмите Ctrl+\\ — программа завершится с подсчётом сигналов\n");
    printf("Ожидаю сигналов...\n");
    fflush(stdout);

    /* Блокируем оба сигнала: тогда между проверкой флага и сном
       ни один SIGQUIT не потеряется (у голого pause() здесь гонка). */
    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    sigaddset(&block, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1) {
        perror("sigprocmask");
        echoctl_restore();
        return EXIT_FAILURE;
    }

    sigemptyset(&waitmask);          /* пустая маска: ждём любые сигналы */
    while (!quit_flag)
        sigsuspend(&waitmask);       /* атомарно: разблокировать и уснуть */

    sigprocmask(SIG_UNBLOCK, &block, NULL);

    echoctl_restore();

    printf("\nВсего прозвучало сигналов: %d\n", (int)beep_count);
    printf("Программа завершена по SIGQUIT\n");

    return EXIT_SUCCESS;
}
