
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_un addr;
    char message[256];
    const char *name = "client";

    const char *texts[] = {
        "hello from client\n",
        "this is a test message\n",
        "asynchronous io is awesome\n"
    };

    if (argc > 1)
        name = argv[1];

    signal(SIGPIPE, SIG_IGN);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, PATH);

    if (connect(fd, (struct sockaddr *)&addr,
                sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        return 1;
    }

    printf("Connected as %s\n", name);

    for (int i = 0; i < 3; i++) {
        size_t len, sent = 0;

        snprintf(message, sizeof(message),
                 "[%s] %s", name, texts[i]);

        len = strlen(message);

        while (sent < len) {
            ssize_t n = write(fd, message + sent, len - sent);

            if (n == -1 && errno == EINTR)
                continue;

            if (n <= 0) {
                perror("write");
                close(fd);
                return 1;
            }

            sent += n;
        }

        printf("Sent: %s", message);
        fflush(stdout);

        struct timespec delay;
        delay.tv_sec = 0;
        delay.tv_nsec = 500000000;
        nanosleep(&delay, NULL);
    }

    close(fd);
    return 0;
}

