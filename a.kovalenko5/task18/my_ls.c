#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

//определение типа файла и прав доступа
void get_permissions(mode_t mode, char *permissions) {
    permissions[0] = S_ISDIR(mode) ? 'd' :
                     S_ISREG(mode) ? '-' : '?';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';
}

//форматирование даты модификации
void get_date(time_t modification_time, char *date, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&modification_time);

    if (tm_info == NULL) {
        snprintf(date, size, "?");
        return;
    }

    if (difftime(now, modification_time) > 15552000 ||
        difftime(now, modification_time) < 0) {
        strftime(date, size, "%b %e %Y", tm_info);
    } else {
        strftime(date, size, "%b %e %H:%M", tm_info);
    }
}

//вывод информации об одном файле
int print_file_info(const char *path) {
    struct stat st;

    if (lstat(path, &st) == -1) {
        perror(path);
        return 1;
    }

    char permissions[11];
    get_permissions(st.st_mode, permissions);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    const char *owner = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char date[32];
    get_date(st.st_mtime, date, sizeof(date));

    char size[32] = "";

    if (S_ISREG(st.st_mode)) {
        snprintf(size, sizeof(size), "%lld", (long long)st.st_size);
    }

    //получение имени файла без пути
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    //вывод информации
    printf("%-10s %4lu %-10s %-10s %10s %-13s %s\n",
           permissions,
           (unsigned long)st.st_nlink,
           owner,
           group,
           size,
           date,
           name);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    int errors = 0;

    for (int i = 1; i < argc; i++) {
        errors |= print_file_info(argv[i]);
    }

    return errors;
}
