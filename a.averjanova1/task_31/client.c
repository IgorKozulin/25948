
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_un addr;
    char text[256];
    const char *name = "client";
    size_t len, sent;

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
    printf("Enter text (empty line to exit):\n");

    while (fgets(text, sizeof(text), stdin) != NULL) {
        char message[512];

        if (text[0] == '\n')
            break;

        snprintf(message, sizeof(message),
                 "[%s] %s", name, text);

        len = strlen(message);
        sent = 0;

        while (sent < len) {
            ssize_t n = write(fd, message + sent,
                              len - sent);

            if (n == -1 && errno == EINTR)
                continue;

            if (n <= 0) {
                perror("write");
                close(fd);
                return 1;
            }

            sent += n;
        }
    }

    close(fd);
    return 0;
}

