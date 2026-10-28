#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void get_permissions(mode_t mode, char *permissions)
{
    if (S_ISDIR(mode))
        permissions[0] = 'd';
    else if (S_ISREG(mode))
        permissions[0] = '-';
    else
        permissions[0] = '?';

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

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s file...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        struct stat st;

        if (lstat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue;
        }

        char permissions[11];
        get_permissions(st.st_mode, permissions);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);

        char date[30];
        time_t now = time(NULL);

        if (now - st.st_mtime > 180 * 24 * 60 * 60) {
            strftime(date, sizeof(date), "%b %e %Y",
                     localtime(&st.st_mtime));
        }
        else {
            strftime(date, sizeof(date), "%b %e %H:%M",
                     localtime(&st.st_mtime));
        }

        char path[1024];
        strncpy(path, argv[i], sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';

        printf("%-10s %4ld %-8s %-8s ",
               permissions,
               (long)st.st_nlink,
               pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?");

        if (S_ISREG(st.st_mode))
            printf("%10lld ", (long long)st.st_size);
        else
            printf("%10s ", "");

        printf("%-12s %s\n", date, basename(path));
    }

    return 0;
}
