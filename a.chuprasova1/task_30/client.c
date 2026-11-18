
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main(void) {
    int sock_fd; //дескпритор клиента
    struct sockaddr_un addr;
    char buffer[256];

    signal(SIGPIPE, SIG_IGN);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("[CLIENT] Клиент запущен. PID = %ld\n", (long)getpid());
    printf("[CLIENT] Подключено к серверу!\n");
    printf("Введите текст (пустая строка - выход):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (strcmp(buffer, "\n") == 0)
            break;

        printf("[CLIENT] Отправляю: %s", buffer);
        fflush(stdout);

        size_t len = strlen(buffer);
        size_t sent = 0;

        while (sent < len) {
            ssize_t n = write(sock_fd, buffer + sent, len - sent);

            if (n == -1 && errno == EINTR)
                continue;

            if (n <= 0) {
                perror("write");
                close(sock_fd);
                return 1;
            }

            sent += (size_t)n;
        }
    }

    close(sock_fd);

    printf("[CLIENT] Соединение закрыто.\n");
    printf("[CLIENT] Программа завершена.\n");

    return 0;
}

