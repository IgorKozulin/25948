#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
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
    int listener, peer;
    struct sockaddr_un local;

    unlink(SOCK_FILE);

    listener = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCK_FILE, sizeof(local.sun_path) - 1);

    if (bind(listener, (struct sockaddr *)&local, sizeof(local)) == -1) {
        perror("bind");
        close(listener);
        return EXIT_FAILURE;
    }

    if (listen(listener, 5) == -1) {
        perror("listen");
        close(listener);
        unlink(SOCK_FILE);
        return EXIT_FAILURE;
    }

    printf("Сервер запущен. Ожидание подключений на %s...\n", SOCK_FILE);
    fflush(stdout);

    do {
        peer = accept(listener, NULL, NULL);
    } while (peer == -1 && errno == EINTR);

    if (peer == -1) {
        perror("accept");
        close(listener);
        unlink(SOCK_FILE);
        return EXIT_FAILURE;
    }

    printf("Клиент подключился!\n");
    fflush(stdout);

    char chunk[256];
    ssize_t got;
    for (;;) {
        got = read(peer, chunk, sizeof(chunk));
        if (got < 0) {
            if (errno == EINTR) continue; 
            perror("read");
            break;
        }
        if (got == 0) break;           

        for (ssize_t k = 0; k < got; k++)
            chunk[k] = (char)toupper((unsigned char)chunk[k]);

        if (send_all(STDOUT_FILENO, chunk, (size_t)got) == -1) {
            perror("write");
            break;
        }
    }

    printf("Клиент отключился. Завершение работы сервера.\n");
    fflush(stdout);

    close(peer);
    close(listener);
    unlink(SOCK_FILE);
    return EXIT_SUCCESS;
}

