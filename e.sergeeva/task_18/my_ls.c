#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void get_mode_string(mode_t mode, char *buf) {
    if (S_ISDIR(mode)) buf[0] = 'd';
    else if (S_ISREG(mode)) buf[0] = '-';
    else buf[0] = '?';
    
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    
    buf[10] = '\0';
}

const char* get_basename(const char *path) {
    const char *last_slash = strrchr(path, '/');
    if (last_slash) return last_slash + 1;
    return path;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [file...]\n", argv[0]);
        return 1;
    }
    
    time_t now = time(NULL);
    
    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            perror(argv[i]);
            continue;
        }
        
        char mode_str[12];
        get_mode_string(st.st_mode, mode_str);
        
        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        
        const char *owner = pw ? pw->pw_name : "?";
        const char *group = gr ? gr->gr_name : "?";
        
        char date_str[32];
        struct tm *tm = localtime(&st.st_mtime);
        double diff = difftime(now, st.st_mtime);
        if (diff > 180.0 * 24 * 60 * 60) {
            strftime(date_str, sizeof(date_str), "%b %e  %Y", tm);
        } else {
            strftime(date_str, sizeof(date_str), "%b %e %H:%M", tm);
        }
        
        char size_str[32];
        if (S_ISREG(st.st_mode)) {
            snprintf(size_str, sizeof(size_str), "%lld", (long long)st.st_size);
        } else {
            size_str[0] = '\0';
        }
        
        const char *name = get_basename(argv[i]);
        
        printf("%-11s %4lu %-8s %-8s %8s %s %s\n",
               mode_str, (unsigned long)st.st_nlink, owner, group,
               size_str, date_str, name);
    }
    
    return 0;
}
