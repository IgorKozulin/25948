
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>
#include <string.h>

void get_permissions(mode_t mode, char *permissions)
{
    // Тип файла
    if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISREG(mode))
        permissions[0] = '-';
    else
        permissions[0] = '?';

    // Права владельца
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Права группы
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Права остальных
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}

int main(int argc, char *argv[])
{
    struct stat st;

    if (argc < 2) {
        fprintf(stderr, "Использование: %s файл1 [файл2 ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {

        // Получаем информацию о файле
        if (lstat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue;
        }

        // Получаем права доступа
        char permissions[11];
        get_permissions(st.st_mode, permissions);

        // Получаем имя владельца
        struct passwd *owner = getpwuid(st.st_uid);
        const char *owner_name = owner ? owner->pw_name : "?";

        // Получаем имя группы
        struct group *group = getgrgid(st.st_gid);
        const char *group_name = group ? group->gr_name : "?";

        // Получаем дату модификации
        char date[64];
        struct tm *tm_info = localtime(&st.st_mtime);

        if (tm_info == NULL) {
            strcpy(date, "unknown");
        } else {
            time_t now = time(NULL);
            double difference = difftime(now, st.st_mtime);

            const char *format;

            if (difference > 6 * 30 * 24 * 60 * 60)
                format = "%b %e %Y";
            else
                format = "%b %e %H:%M";

            strftime(date, sizeof(date), format, tm_info);
        }

        // Получаем имя файла без пути
        char *path_copy = malloc(strlen(argv[i]) + 1);

        if (path_copy == NULL) {
            perror("malloc");
            continue;
        }

        strcpy(path_copy, argv[i]);

        char *filename = basename(path_copy);

        // Размер только для обычных файлов
        char size[32];

        if (S_ISREG(st.st_mode))
            snprintf(size, sizeof(size), "%lld",
                     (long long)st.st_size);
        else
            size[0] = '\0';

        // Вывод информации
        printf("%-10s %3lu %-10s %-10s %10s %-12s %s\n",
               permissions,
               (unsigned long)st.st_nlink,
               owner_name,
               group_name,
               size,
               date,
               filename);

        free(path_copy);
    }

    return 0;
}

