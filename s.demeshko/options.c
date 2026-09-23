#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <sys/resource.h>

extern char **environ;

struct opt_rec {
    int opt;
    char *arg;
};

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:";
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
        case '?':
        default:
            if (optopt) fprintf(stderr, "Unknown option: -%c\n", optopt);
            break;
        }
    }

    for (i = 0; i < count; i++) free(list[i].arg);
    free(list);
    return 0;
}
