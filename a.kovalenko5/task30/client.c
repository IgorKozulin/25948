
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

static const char socket_path[] = "./socket";

static int write_all(int fd, const char *buf, size_t size)
{
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = write(fd, buf + sent, size - sent);

        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }

        if (n == 0) {
            errno = EIO;
            return -1;
        }

        sent += (size_t)n;
    }
    return 0;
}

int main(void)
{
    int sock_fd;
    struct sockaddr_un addr;
    char buffer[1024];
    int status = EXIT_SUCCESS;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sock_fd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path),
             "%s", socket_path);

    if (connect(sock_fd, (struct sockaddr *)&addr,
                sizeof(addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("Соединение установлено.\n");
    printf("Введите текст и нажмите Enter.\n");
    printf("Для завершения нажмите Ctrl+D на пустой строке.\n");
    fflush(stdout);

    for (;;) {
        ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));

        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read stdin");
            status = EXIT_FAILURE;
            break;
        }

        if (n == 0) break;

        if (write_all(sock_fd, buffer, (size_t)n) < 0) {
            perror("write socket");
            status = EXIT_FAILURE;
            break;
        }
    }

    close(sock_fd);
    printf("Клиент завершил работу.\n");

    return status;
}

