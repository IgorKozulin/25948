#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void get_perms(mode_t mode, char *perms) {
    perms[10] = '\0';
    perms[0] = S_ISDIR(mode) ? 'd' : (S_ISREG(mode) ? '-' : '?');
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue;
        }

        char perms[11];
        get_perms(st.st_mode, perms);

        struct passwd *pw = getpwuid(st.st_uid);
        const char *owner = pw ? pw->pw_name : "?";

        struct group *gr = getgrgid(st.st_gid);
        const char *group = gr ? gr->gr_name : "?";

        char size_str[20];
        if (S_ISREG(st.st_mode)) {
            snprintf(size_str, sizeof(size_str), "%lld", (long long)st.st_size);
        } else {
            snprintf(size_str, sizeof(size_str), " ");
        }

        time_t now = time(NULL);
        struct tm *file_tm = localtime(&st.st_mtime);
        char date_str[20];
        long long diff = now - st.st_mtime;
        
        if (diff > 15552000LL || diff < 0) {
            strftime(date_str, sizeof(date_str), "%b %e  %Y", file_tm);
        } else {
            strftime(date_str, sizeof(date_str), "%b %e %H:%M", file_tm);
        }

        char *path_copy = strdup(argv[i]);
        char *fname = basename(path_copy);

        printf("%-10s %4ld %-8s %-8s %8s %12s %s\n", 
               perms, st.st_nlink, owner, group, size_str, date_str, fname);

        free(path_copy);
    }
    return 0;
}
