/* Задача 30, сервер: AF_UNIX/SOCK_STREAM, вывод полученного текста в верхнем регистре */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"      /* должен совпадать с client.c */

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

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[4096];
    ssize_t n;

    signal(SIGPIPE, SIG_IGN);       /* клиент может умереть раньше нас */

    unlink(SOCKET_PATH);            /* остаток файла от прошлого запуска */

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof addr);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof addr.sun_path - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, (socklen_t)sizeof addr) == -1) {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("Сервер запущен. Ожидание подключений на %s...\n", SOCKET_PATH);
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("Клиент подключился!\n");
    fflush(stdout);

    for (;;) {
        n = read(client_fd, buf, sizeof buf);
        if (n == -1) {
            if (errno == EINTR) continue;
            perror("read");
            break;
        }
        if (n == 0)
            break;                  /* клиент закрыл соединение */

        for (ssize_t i = 0; i < n; i++)
            buf[i] = (char)toupper((unsigned char)buf[i]);

        if (write_all(STDOUT_FILENO, buf, (size_t)n) == -1) {
            perror("write");
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);            /* не засоряем каталог */

    printf("Клиент отключился. Завершение работы сервера.\n");
    return EXIT_SUCCESS;
}
