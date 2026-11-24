/* Задача 18: my_ls — аналог ls -ld */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <libgen.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define SIX_MONTHS (6L * 30L * 24L * 60L * 60L)

struct entry {
    const char *path;
    struct stat st;
    int  ok;
    char mode[11];          /* "-rw-r--r--" + '\0' */
    char owner[64];
    char group[64];
    char size[32];          /* пусто для не-обычных файлов */
    char date[32];
    char name[256];
};

static void fill_mode(mode_t m, char out[11])
{
    /* по заданию: d — каталог, - — обычный файл, ? — всё остальное */
    out[0] = S_ISDIR(m) ? 'd' : (S_ISREG(m) ? '-' : '?');
    out[1] = (m & S_IRUSR) ? 'r' : '-';
    out[2] = (m & S_IWUSR) ? 'w' : '-';
    out[3] = (m & S_IXUSR) ? 'x' : '-';
    out[4] = (m & S_IRGRP) ? 'r' : '-';
    out[5] = (m & S_IWGRP) ? 'w' : '-';
    out[6] = (m & S_IXGRP) ? 'x' : '-';
    out[7] = (m & S_IROTH) ? 'r' : '-';
    out[8] = (m & S_IWOTH) ? 'w' : '-';
    out[9] = (m & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

static void fill_owner(struct entry *e)
{
    struct passwd *pw = getpwuid(e->st.st_uid);
    if (pw != NULL && pw->pw_name != NULL)
        snprintf(e->owner, sizeof e->owner, "%s", pw->pw_name);
    else
        snprintf(e->owner, sizeof e->owner, "%lu",
                 (unsigned long)e->st.st_uid);   /* нет в /etc/passwd */

    struct group *gr = getgrgid(e->st.st_gid);
    if (gr != NULL && gr->gr_name != NULL)
        snprintf(e->group, sizeof e->group, "%s", gr->gr_name);
    else
        snprintf(e->group, sizeof e->group, "%lu",
                 (unsigned long)e->st.st_gid);
}

static void fill_date(time_t mtime, char *out, size_t outsz)
{
    struct tm tm_file;
    time_t now = time(NULL);

    if (localtime_r(&mtime, &tm_file) == NULL) {
        snprintf(out, outsz, "?");
        return;
    }

    /* как ls: старые и «из будущего» — с годом, свежие — с временем */
    if (mtime > now + 60 || now - mtime > SIX_MONTHS)
        strftime(out, outsz, "%b %e  %Y", &tm_file);
    else
        strftime(out, outsz, "%b %e %H:%M", &tm_file);
}

static int max_int(int a, int b) { return a > b ? a : b; }

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s path...\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = argc - 1;
    struct entry *ents = calloc((size_t)n, sizeof *ents);
    if (ents == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    int rc = EXIT_SUCCESS;

    /* проход 1: собрать данные */
    for (int i = 0; i < n; i++) {
        ents[i].path = argv[i + 1];

        if (lstat(ents[i].path, &ents[i].st) == -1) {
            fprintf(stderr, "%s: %s\n", ents[i].path, strerror(errno));
            rc = EXIT_FAILURE;
            continue;                       /* этот аргумент просто пропускаем */
        }
        ents[i].ok = 1;

        fill_mode(ents[i].st.st_mode, ents[i].mode);
        fill_owner(&ents[i]);
        fill_date(ents[i].st.st_mtime, ents[i].date, sizeof ents[i].date);

        if (S_ISREG(ents[i].st.st_mode))
            snprintf(ents[i].size, sizeof ents[i].size, "%lld",
                     (long long)ents[i].st.st_size);
        else
            ents[i].size[0] = '\0';         /* размер только для обычных файлов */

        char tmp[PATH_MAX];
        snprintf(tmp, sizeof tmp, "%s", ents[i].path);
        snprintf(ents[i].name, sizeof ents[i].name, "%s", basename(tmp));
    }

    /* проход 2а: ширины колонок по всем строкам */
    int w_link = 1, w_owner = 1, w_group = 1, w_size = 1;

    for (int i = 0; i < n; i++) {
        if (!ents[i].ok) continue;
        char buf[32];
        snprintf(buf, sizeof buf, "%lu", (unsigned long)ents[i].st.st_nlink);
        w_link  = max_int(w_link,  (int)strlen(buf));
        w_owner = max_int(w_owner, (int)strlen(ents[i].owner));
        w_group = max_int(w_group, (int)strlen(ents[i].group));
        w_size  = max_int(w_size,  (int)strlen(ents[i].size));
    }

    /* проход 2б: печать таблицы */
    for (int i = 0; i < n; i++) {
        if (!ents[i].ok) continue;
        printf("%-10s %*lu %-*s %-*s %*s %s %s\n",
               ents[i].mode,
               w_link,  (unsigned long)ents[i].st.st_nlink,
               w_owner, ents[i].owner,
               w_group, ents[i].group,
               w_size,  ents[i].size,
               ents[i].date,
               ents[i].name);
    }

    free(ents);
    return rc;                              /* 0, если все аргументы обработаны */
}
