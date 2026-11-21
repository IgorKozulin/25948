
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define PATH "./socket"

int main(void)
{
    int fd;
    struct sockaddr_un addr;
    char text[256];

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

    printf("Connected to server!\n");
    printf("Enter text (empty line to exit):\n");

    while (fgets(text, sizeof(text), stdin) != NULL) {
        if (text[0] == '\n')
            break;

        size_t len = strlen(text);
        size_t sent = 0;

        while (sent < len) {
            ssize_t n = write(fd, text + sent,
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

