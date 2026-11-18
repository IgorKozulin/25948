#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SOCKET_PATH "./socket"

int main(void) {
    int sock_fd;
    struct sockaddr_un addr;
    const char *messages[] = {
        "Hello World!\n",
        "This is a Test Message!!!\n",
        "Unix domain sockets are awesome.\n"
    };
    int num_messages = sizeof(messages) / sizeof(messages[0]);
    int i;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket error");
        return EXIT_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    printf("[CLIENT] Connected to server.\n");

    for (i = 0; i < num_messages; i++) {
        printf("[CLIENT] Sending: %s", messages[i]);
        if (write(sock_fd, messages[i], strlen(messages[i])) == -1) {
            perror("write error");
            break;
        }
        sleep(1);
    }

    printf("[CLIENT] Done sending. Closing socket.\n");
    close(sock_fd);

    return EXIT_SUCCESS;
}
