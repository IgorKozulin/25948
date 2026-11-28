/* Задача 31: клиент — отправляет несколько сообщений серверу */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

static const char *messages[] = {
    "Hello from client!",
    "This is a test message.",
    "Goodbye!",
};

static void sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static int write_all(int fd, const char *buf, size_t len)
{
    size_t done = 0;

    while (done < len) {
        ssize_t n = write(fd, buf + done, len - done);
        if (n == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        done += (size_t)n;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *name = (argc > 1) ? argv[1] : "Client";
    int fd;
    struct sockaddr_un addr;
    char line[512];

    signal(SIGPIPE, SIG_IGN);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof addr.sun_path - 1);

    if (connect(fd, (struct sockaddr *)&addr, (socklen_t)sizeof addr) == -1) {
        perror("connect");
        fprintf(stderr, "Сервер не запущен? Ожидался %s\n", SOCKET_PATH);
        close(fd);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "[%s] Подключено, отправляю сообщения...\n", name);

    for (size_t i = 0; i < sizeof messages / sizeof messages[0]; i++) {
        int len = snprintf(line, sizeof line, "[%s] %s\n", name, messages[i]);
        if (len < 0 || (size_t)len >= sizeof line) {
            fprintf(stderr, "[%s] слишком длинная строка\n", name);
            close(fd);
            return EXIT_FAILURE;
        }
        if (write_all(fd, line, (size_t)len) == -1) {
            perror("write");
            close(fd);
            return EXIT_FAILURE;
        }
        fprintf(stderr, "[%s] отправил: %s", name, line);
        sleep_ms(300);          /* чтобы сообщения клиентов чередовались */
    }

    close(fd);
    fprintf(stderr, "[%s] Готово, соединение закрыто.\n", name);
    return EXIT_SUCCESS;
}
