#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <time.h>
#include <sys/resource.h>

extern char **environ;

struct opt_rec {
    int opt;
    char *arg;
};

/* Ручное время, если пользователь передал -T HH:MM */
static char *manual_time = NULL;

/* Проверка: строка похожа на время (H:MM, HH:MM, HH:MM:SS) */
int is_time_arg(const char *s) {
    int len = (int)strlen(s);
    /* H:MM */
    if (len == 4 && s[1] == ':' && isdigit(s[0]) && isdigit(s[2]) && isdigit(s[3]))
        return (s[0]-'0') < 24 && (s[2]-'0')*10 + (s[3]-'0') < 60;
    /* HH:MM */
    if (len == 5 && s[2] == ':' && isdigit(s[0]) && isdigit(s[1])
        && isdigit(s[3]) && isdigit(s[4]))
        return (s[0]-'0')*10 + (s[1]-'0') < 24
            && (s[3]-'0')*10 + (s[4]-'0') < 60;
    /* HH:MM:SS */
    if (len == 8 && s[2] == ':' && s[5] == ':'
        && isdigit(s[0]) && isdigit(s[1]) && isdigit(s[3])
        && isdigit(s[4]) && isdigit(s[6]) && isdigit(s[7]))
        return (s[0]-'0')*10 + (s[1]-'0') < 24
            && (s[3]-'0')*10 + (s[4]-'0') < 60
            && (s[6]-'0')*10 + (s[7]-'0') < 60;
    return 0;
}

/* Вывод времени: ручное или реальное */
void print_time(void) {
    if (manual_time) {
        printf("-t: %s\n", manual_time);
        return;
    }
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    if (!lt) { perror("localtime"); return; }
    const char *zone = tzname[lt->tm_isdst > 0 ? 1 : 0];
    printf("-t: %02d/%02d/%04d %02d:%02d %s\n",
           lt->tm_mon + 1, lt->tm_mday, lt->tm_year + 1900,
           lt->tm_hour, lt->tm_min, zone);
}

int main(int argc, char *argv[]) {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    const char *optstring = "ispuU:cC:dvV:tT:";
    struct opt_rec *list = NULL;
    int count = 0;
    int c, i;

    while ((c = getopt(argc, argv, optstring)) != -1) {
        struct opt_rec *tmp = realloc(list, sizeof(struct opt_rec) * (count + 1));
        if (!tmp) { perror("realloc"); free(list); return 1; }
        list = tmp;
        list[count].opt = c;
        list[count].arg = optarg ? strdup(optarg) : NULL;
        if (optarg && !list[count].arg) { perror("strdup"); return 1; }
        count++;
    }

    for (i = count - 1; i >= 0; i--) {
        switch (list[i].opt) {
        case 'i':
            printf("-i: real UID=%d, effective UID=%d, real GID=%d, effective GID=%d\n",
                   (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());
            break;
        case 's':
            if (setpgid(0, 0) == -1) perror("-s: setpgid");
            else printf("-s: PGID=%d\n", (int)getpgrp());
            break;
        case 'p':
            printf("-p: PID=%d, PPID=%d, PGID=%d\n",
                   (int)getpid(), (int)getppid(), (int)getpgrp());
            break;
        case 'u': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
                printf("-u: NOFILE soft=%lu hard=%lu\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
            else perror("-u: getrlimit");
            break;
        }
        case 'U': {
            char *end = NULL;
            errno = 0;
            long val = strtol(list[i].arg, &end, 10);
            if (errno || end == list[i].arg || *end != '\0' || val < 0) {
                fprintf(stderr, "-U: bad value '%s'\n", list[i].arg);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) { perror("-U: getrlimit"); break; }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_NOFILE, &rl) == -1) perror("-U: setrlimit");
            else printf("-U: NOFILE set to %ld\n", val);
            break;
        }
        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0)
                printf("-c: CORE soft=%lu hard=%lu\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
            else perror("-c: getrlimit");
            break;
        }
        case 'C': {
            char *end = NULL;
            errno = 0;
            long val = strtol(list[i].arg, &end, 10);
            if (errno || end == list[i].arg || *end != '\0' || val < 0) {
                fprintf(stderr, "-C: bad value '%s'\n", list[i].arg);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) { perror("-C: getrlimit"); break; }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_CORE, &rl) == -1) perror("-C: setrlimit");
            else printf("-C: CORE set to %ld\n", val);
            break;
        }
        case 'd': {
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf)) != NULL) printf("-d: cwd=%s\n", buf);
            else perror("-d: getcwd");
            break;
        }
        case 'v':
            printf("-v: environment:\n");
            for (char **e = environ; *e; e++) printf("  %s\n", *e);
            break;
        case 'V':
            if (putenv(list[i].arg) == 0) printf("-V: set '%s'\n", list[i].arg);
            else perror("-V: putenv");
            break;
        case 't':
            print_time();
            break;
        case 'T':
            if (is_time_arg(list[i].arg)) {
                /* Ручное время HH:MM */
                free(manual_time);
                manual_time = strdup(list[i].arg);
                if (!manual_time) { perror("strdup"); break; }
                printf("-T: время установлено в '%s'\n", manual_time);
            } else {
                /* Часовой пояс */
                if (setenv("TZ", list[i].arg, 1) == 0) {
                    tzset();
                    printf("-T: TZ set to '%s'\n", list[i].arg);
                } else perror("-T: setenv");
            }
            break;
        case '?':
        default:
            if (optopt) fprintf(stderr, "Unknown option: -%c\n", optopt);
            break;
        }
    }

    free(manual_time);
    for (i = 0; i < count; i++) free(list[i].arg);
    free(list);
    return 0;
}
