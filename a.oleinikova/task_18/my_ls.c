#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>


static void usage(const char *prog);
static void format_t_r(mode_t mode, char out[11]);
static const char *owner_name(uid_t uid);
static const char *group_name(gid_t gid);
static void format_date(time_t mtime, char *out, size_t outsz);
static const char *base_name(const char *path);


int main(int argc, char *argv[]){
    if (argc < 2)
        usage(argv[0]);

    for (int i = 1; i < argc; i++){
        struct stat st;
        if (lstat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue;
        }

        
        char t_and_r[11]; // тип + права
        format_t_r(st.st_mode, t_and_r);

        // влаелец и группа
        const char *owner = owner_name(st.st_uid);
        const char *group = group_name(st.st_gid);

        //размер (для об файлов "-"), "d" - каталог, "?" - все остальное
        char size_str[32];
        if (S_ISREG(st.st_mode))
            snprintf(size_str, sizeof size_str, "%lld", (long long)st.st_size);
        else
            size_str[0] = '\0';   

        // дата
        char date_str[64];
        format_date(st.st_mtime, date_str, sizeof date_str);

        // имя (basename)
        const char *name = base_name(argv[i]);

        
        printf("%s %3lu %-8s %-8s %8s %s %s\n", t_and_r,(unsigned long)st.st_nlink,  owner,    group,  size_str,   date_str,   name);
    }

    return 0;
}





static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", prog);
    exit(EXIT_FAILURE);
}


static void format_t_r(mode_t mode, char out[11]){
    
    if (S_ISDIR(mode))      out[0] = 'd';
    else if (S_ISREG(mode)) out[0] = '-';
    else                    out[0] = '?';

    
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';

    
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';

    
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';

    out[10] = '\0';
}


static const char *owner_name(uid_t uid){
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : "?";
}

static const char *group_name(gid_t gid)
{
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : "?";
}


static void format_date(time_t mtime, char *out, size_t outsz){
    struct tm *tm = localtime(&mtime);
    if (!tm) { snprintf(out, outsz, "?"); return; }

    time_t now = time(NULL);
    double age = difftime(now, mtime);

    
    const double SIX_MONTHS = 180.0 * 24 * 3600; // 6 месяцев

    if (age > SIX_MONTHS || age < -3600)  // старый файл или время не коректно
        strftime(out, outsz, "%b %e  %Y", tm);
    else
        strftime(out, outsz, "%b %e %H:%M", tm);
}


static const char *base_name(const char *path){
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}