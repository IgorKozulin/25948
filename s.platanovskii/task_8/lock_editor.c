#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file> <shared|exclusive>\n", argv[0]);
        return 1;
    }

    int lock_type = (strcmp(argv[2], "shared") == 0) ? F_RDLCK : F_WRLCK;
    int fd = open(argv[1], O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct flock fl = {0};
    fl.l_type = lock_type;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    printf("Ждем блокировку файла '%s'...\n", argv[1]);
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        perror("fcntl lock");
        close(fd);
        return 1;
    }

    printf("Файл заблокирован! Запуск редактора...\n");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "nano %s", argv[1]);
    system(cmd);

    printf("Редактор закрыт, снимаем блокировку.\n");
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);
    
    close(fd);
    return 0;
}
