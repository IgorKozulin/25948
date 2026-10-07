#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>

extern char **environ;

#define MAX_ACTIONS 128

struct action {
    int   opt;
    char *arg;
};

static struct action actions[MAX_ACTIONS];
static int n_actions = 0;

static void add_action(int opt, char *arg) {
    if (n_actions < MAX_ACTIONS) {
        actions[n_actions].opt = opt;
        actions[n_actions].arg = arg;
        n_actions++;
    }
}

static void do_i(void) {
    printf("UID=%d EUID=%d GID=%d EGID=%d\n",
           (int)getuid(), (int)geteuid(),
           (int)getgid(), (int)getegid());
}

static void do_s(void) {
    if (setpgid(0, 0) != 0)
        perror("setpgid");
    else
        printf("Became process group leader, PGID=%d\n", (int)getpgrp());
}

static void do_p(void) {
    printf("PID=%d PPID=%d PGID=%d\n",
           (int)getpid(), (int)getppid(), (int)getpgrp());
}

static void do_u(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
        perror("getrlimit");
        return;
    }
    printf("ulimit (RLIMIT_NOFILE): soft=%ld hard=%ld\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
}

static void do_c(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) != 0) {
        perror("getrlimit");
        return;
    }
    printf("core size (RLIMIT_CORE): soft=%ld hard=%ld\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
}

static void do_d(void) {
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != NULL)
        printf("CWD: %s\n", buf);
    else
        perror("getcwd");
}

static void do_v(void) {
    for (char **p = environ; *p != NULL; p++)
        printf("%s\n", *p);
}

static long parse_long(const char *optname, const char *arg, int *ok) {
    char *end;
    long val;
    errno = 0;
    val = strtol(arg, &end, 10);
    if (errno != 0 || *end != '\0' || val < 0) {
        fprintf(stderr, "%s: invalid value '%s'\n", optname, arg);
        *ok = 0;
        return 0;
    }
    *ok = 1;
    return val;
}

static void do_U(const char *arg) {
    int ok;
    long val = parse_long("-U", arg, &ok);
    if (!ok) return;

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
        perror("getrlimit");
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_NOFILE, &rl) != 0)
        perror("setrlimit");
    else
        printf("ulimit changed to %ld\n", val);
}

static void do_C(const char *arg) {
    int ok;
    long val = parse_long("-C", arg, &ok);
    if (!ok) return;

    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) != 0) {
        perror("getrlimit");
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_CORE, &rl) != 0)
        perror("setrlimit");
    else
        printf("core size changed to %ld\n", val);
}

static void do_V(const char *arg) {
    if (putenv((char *)arg) != 0)
        perror("putenv");
    else
        printf("env set: %s\n", arg);
}

int main(int argc, char *argv[]) {
    char *options = "ispuU:cC:dvV:";
    int c;

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
            case 'i': case 's': case 'p': case 'u':
            case 'c': case 'd': case 'v':
                add_action(c, NULL);
                break;

            case 'U': case 'C': case 'V':
                add_action(c, optarg);
                break;

            case '?':
                fprintf(stderr, "Invalid option: %c\n", optopt);
                break;
        }
    }

    for (int i = n_actions - 1; i >= 0; i--) {
        switch (actions[i].opt) {
            case 'i': do_i(); break;
            case 's': do_s(); break;
            case 'p': do_p(); break;
            case 'u': do_u(); break;
            case 'U': do_U(actions[i].arg); break;
            case 'c': do_c(); break;
            case 'C': do_C(actions[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V(actions[i].arg); break;
        }
    }

    return 0;
}
