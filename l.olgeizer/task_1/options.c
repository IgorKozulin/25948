#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

struct action {
    char opt;
    char *arg;
};

static void do_i(void) {
    printf("UID=%ld, EUID=%ld, GID=%ld, EGID=%ld\n",
           (long)getuid(), (long)geteuid(),
           (long)getgid(), (long)getegid());
}

static void do_s(void) {
    if (setpgid(0, 0) == -1)
        perror("setpgid");
    else
        printf("Process became group leader. PGID=%ld\n", (long)getpgrp());
}

static void do_p(void) {
    printf("PID=%ld, PPID=%ld, PGID=%ld\n",
           (long)getpid(), (long)getppid(), (long)getpgrp());
}

static void do_u(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) { perror("getrlimit"); return; }
    printf("ulimit (RLIMIT_NOFILE) soft=%ld hard=%ld\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
}

static void do_U(const char *arg) {
    if (!arg) { fprintf(stderr, "-U requires a value\n"); return; }
    errno = 0;
    char *end = NULL;
    long val = strtol(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' || val < 0) {
        fprintf(stderr, "Invalid value for -U: %s\n", arg);
        return;
    }
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) { perror("getrlimit"); return; }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
        perror("setrlimit(RLIMIT_NOFILE)");
    else
        printf("ulimit set to %ld\n", val);
}

static void do_c(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) { perror("getrlimit"); return; }
    printf("core file size soft=%ld bytes, hard=%ld bytes\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
}

static void do_C(const char *arg) {
    if (!arg) { fprintf(stderr, "-C requires a value\n"); return; }
    errno = 0;
    char *end = NULL;
    long val = strtol(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' || val < 0) {
        fprintf(stderr, "Invalid value for -C: %s\n", arg);
        return;
    }
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) { perror("getrlimit"); return; }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_CORE, &rl) == -1)
        perror("setrlimit(RLIMIT_CORE)");
    else
        printf("core size set to %ld bytes\n", val);
}

static void do_d(void) {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL)
        perror("getcwd");
    else
        printf("cwd: %s\n", buf);
}

static void do_v(void) {
    for (char **e = environ; *e; e++)
        printf("%s\n", *e);
}

static void do_V(const char *arg) {
    if (!arg || strchr(arg, '=') == NULL) {
        fprintf(stderr, "-V requires NAME=value\n");
        return;
    }
    char *copy = strdup(arg);
    if (!copy) { perror("strdup"); return; }
    if (putenv(copy) != 0)
        perror("putenv");
    else
        printf("Environment variable set: %s\n", arg);
}

int main(int argc, char *argv[]) {
    static struct action actions[128];
    int n = 0;
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (c == '?') {
            fprintf(stderr, "Invalid option: -%c\n", optopt);
            continue;
        }
        if (n < (int)(sizeof(actions)/sizeof(actions[0]))) {
            actions[n].opt = (char)c;
            actions[n].arg = optarg;
            n++;
        }
    }

    /* Выполняем в порядке справа налево */
    for (int i = n - 1; i >= 0; i--) {
        switch (actions[i].opt) {
            case 'i': do_i();         break;
            case 's': do_s();         break;
            case 'p': do_p();         break;
            case 'u': do_u();         break;
            case 'U': do_U(actions[i].arg); break;
            case 'c': do_c();         break;
            case 'C': do_C(actions[i].arg); break;
            case 'd': do_d();         break;
            case 'v': do_v();         break;
            case 'V': do_V(actions[i].arg); break;
        }
    }
    return 0;
}
