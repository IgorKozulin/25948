#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_FILE "./socket"

static int send_all(int fd, const char *data, size_t count) {
    while (count > 0) {
        ssize_t done = write(fd, data, count);
        if (done < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        data += done;
        count -= (size_t)done;
    }
    return 0;
}

int main(void) {
    int conn;
    struct sockaddr_un remote;

    signal(SIGPIPE, SIG_IGN);

    conn = socket(AF_UNIX, SOCK_STREAM, 0);
    if (conn == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_FILE, sizeof(remote.sun_path) - 1);

    if (connect(conn, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
        perror("connect (сервер запущен?)");
        close(conn);
        return EXIT_FAILURE;
    }

    printf("Подключено к серверу. Отправка сообщений...\n");

    const char *texts[] = {
        "Hello World!\n",
        "This is a Test Message!!!\n",
        "Unix domain sockets are awesome.\n",
    };
    size_t total = sizeof(texts) / sizeof(texts[0]);

    for (size_t k = 0; k < total; k++) {
        printf("[CLIENT] Отправляю: %.*s\n",
               (int)(strlen(texts[k]) - 1), texts[k]);  // без завершающего \n
        if (send_all(conn, texts[k], strlen(texts[k])) == -1) {
            if (errno == EPIPE)
                fprintf(stderr, "[CLIENT] Сервер закрыл соединение\n");
            else
                perror("write");
            close(conn);
            return EXIT_FAILURE;
        }
    }

    printf("[CLIENT] Все сообщения отправлены. Закрытие соединения...\n");

    close(conn);
    return EXIT_SUCCESS;
}

