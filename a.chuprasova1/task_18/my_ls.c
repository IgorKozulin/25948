
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void get_permissions(mode_t mode, char *permissions) {  // определения файла и правав доступа

    permissions[0] = S_ISDIR(mode) ? 'd' : (S_ISREG(mode) ? '-' : '?'); // директория, файл

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-'; // чтение    владелец
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-'; // запись
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-'; //выполнение

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';  // группа
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-'; // остальные
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file...\n", argv[0]);
        return 1;
    }

    int error = 0;

    for (int i = 1; i < argc; i++) {  // перебор файлов
        struct stat st;

        if (lstat(argv[i], &st) == -1) { //получаем инф
            perror(argv[i]);
            error = 1;
            continue;
        }

        char permissions[11]; //буфер для прав
        get_permissions(st.st_mode, permissions);

        struct passwd *pw = getpwuid(st.st_uid); // имя владельца
        struct group *gr = getgrgid(st.st_gid); //имя группы

        char owner[32], group[32];

        if (pw != NULL) 
            snprintf(owner, sizeof(owner), "%s", pw->pw_name); //текс в массив
        else
            snprintf(owner, sizeof(owner), "%u", (unsigned int)st.st_uid); //uid

        if (gr != NULL)
            snprintf(group, sizeof(group), "%s", gr->gr_name);
        else
            snprintf(group, sizeof(group), "%u", (unsigned int)st.st_gid);

        char date[32];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&st.st_mtime);

        if (tm_info != NULL) {
            if (st.st_mtime > now ||
                difftime(now, st.st_mtime) > 15552000) { 
                strftime(date, sizeof(date), "%b %e %Y", tm_info);
            } else {
                strftime(date, sizeof(date), "%b %e %H:%M", tm_info);
            }
        } else {
            snprintf(date, sizeof(date), "?");
        }

        char size[32] = "";

        if (S_ISREG(st.st_mode)) { //тип файла
            snprintf(size, sizeof(size), "%lld",
                     (long long)st.st_size);
        }

        const char *name = strrchr(argv[i], '/');

        if (name != NULL)
            name++;
        else
            name = argv[i];

        if (*name == '\0')
            name = argv[i];

        printf("%-10s %4llu %-12s %-12s %10s %-12s %s\n",
               permissions,
               (unsigned long long)st.st_nlink,
               owner,
               group,
               size,
               date,
               name);
    }

    return error;
}

