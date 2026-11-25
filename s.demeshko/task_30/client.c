/* Задача 30, клиент: подключается к серверу и передаёт ему текст */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"      /* должен совпадать с server.c */

static const char *messages[] = {
    "Hello World!",
    "This is a Test Message!!!",
    "Unix domain sockets are awesome.",
};

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
    int fd;
    struct sockaddr_un addr;

    signal(SIGPIPE, SIG_IGN);       /* закрытие сервером не должно убивать нас */

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
        fprintf(stderr, "Сервер не запущен? Ожидался файл %s\n", SOCKET_PATH);
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Подключено к серверу. Отправка сообщений...\n");
    fflush(stdout);

    for (size_t i = 0; i < sizeof messages / sizeof messages[0]; i++) {
        printf("[CLIENT] Отправляю: %s\n", messages[i]);
        fflush(stdout);

        if (write_all(fd, messages[i], strlen(messages[i])) == -1 ||
            write_all(fd, "\n", 1) == -1) {
            if (errno == EPIPE)
                fprintf(stderr, "[CLIENT] Сервер закрыл соединение\n");
            else
                perror("[CLIENT] write");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    printf("[CLIENT] Все сообщения отправлены. Закрытие соединения...\n");
    fflush(stdout);

    close(fd);                      /* сервер получит EOF и завершится */
    return EXIT_SUCCESS;
}
