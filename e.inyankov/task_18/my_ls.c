#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void print_file_info(const char *path) {
    struct stat st;
    char path_copy[1024];
    char mode_str[11];
    char user_name[32];
    char group_name[32];
    char size_str[32];
    char time_str[32];
    struct passwd *pw;
    struct group *gr;
    struct tm *tm_info;
    time_t now;

    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        mode_str[0] = 'd';
    } else if (S_ISREG(st.st_mode)) {
        mode_str[0] = '-';
    } else {
        mode_str[0] = '?';
    }

    mode_str[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    mode_str[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    mode_str[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    mode_str[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    mode_str[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    mode_str[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    mode_str[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    mode_str[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    mode_str[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    mode_str[10] = '\0';

    pw = getpwuid(st.st_uid);
    if (pw != NULL) {
        snprintf(user_name, sizeof(user_name), "%s", pw->pw_name);
    } else {
        snprintf(user_name, sizeof(user_name), "%ld", (long)st.st_uid);
    }

    gr = getgrgid(st.st_gid);
    if (gr != NULL) {
        snprintf(group_name, sizeof(group_name), "%s", gr->gr_name);
    } else {
        snprintf(group_name, sizeof(group_name), "%ld", (long)st.st_gid);
    }

    if (S_ISREG(st.st_mode)) {
        snprintf(size_str, sizeof(size_str), "%lld", (long long)st.st_size);
    } else {
        size_str[0] = '\0';
    }

    now = time(NULL);
    tm_info = localtime(&st.st_mtime);
    if (tm_info != NULL) {
        if (now - st.st_mtime > 180 * 24 * 60 * 60 || st.st_mtime > now) {
            strftime(time_str, sizeof(time_str), "%b %e  %Y", tm_info);
        } else {
            strftime(time_str, sizeof(time_str), "%b %e %H:%M", tm_info);
        }
    } else {
        snprintf(time_str, sizeof(time_str), "??? ?? ??:??");
    }

    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    printf("%s %3ld %-8s %-8s %8s %s %s\n",
           mode_str,
           (long)st.st_nlink,
           user_name,
           group_name,
           size_str,
           time_str,
           basename(path_copy));
}

int main(int argc, char *argv[]) {
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_or_dir>...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return EXIT_SUCCESS;
}
