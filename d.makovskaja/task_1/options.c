#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} Action;

int main(int argc, char *argv[]) {
    Action actions[argc];
    int action_count = 0;
    int c;
    char *optstring = "ispuU:cC:dvV:";

    while ((c = getopt(argc, argv, optstring)) != -1) {
        if (c == '?') return 1;
        actions[action_count].opt = c;
        actions[action_count].arg = optarg;
        action_count++;
    }

    for (int i = action_count - 1; i >= 0; i--) {
        char opt = actions[i].opt;
        char *arg = actions[i].arg;

        switch (opt) {
            case 'i':
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
            case 's':
                if (setpgid(0, 0) != 0) perror("setpgid error");
                else printf("Process became group leader.\n");
                break;
            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgid(0));
                break;
            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
                    printf("Current ulimit (NOFILE): %ld\n", (long)rl.rlim_cur);
                else perror("getrlimit error");
                break;
            }
            case 'U': {
                char *endptr;
                long val = strtol(arg, &endptr, 10);
                if (*endptr != '\0' || val < 0)
                    fprintf(stderr, "Invalid value for -U: %s\n", arg);
                else {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                        rl.rlim_cur = val;
                        if (val > rl.rlim_max) rl.rlim_max = val;
                        if (setrlimit(RLIMIT_NOFILE, &rl) != 0) perror("setrlimit error");
                        else printf("ulimit changed to %ld\n", val);
                    }
                }
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0)
                    printf("Max core file size: %ld bytes\n", (long)rl.rlim_cur);
                else perror("getrlimit error (CORE)");
                break;
            }
            case 'C': {
                char *endptr;
                long val = strtol(arg, &endptr, 10);
                if (*endptr != '\0' || val < 0)
                    fprintf(stderr, "Invalid value for -C: %s\n", arg);
                else {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                        rl.rlim_cur = val;
                        if (val > rl.rlim_max) rl.rlim_max = val;
                        if (setrlimit(RLIMIT_CORE, &rl) != 0) perror("setrlimit error (CORE)");
                        else printf("Core file size changed to %ld bytes\n", val);
                    }
                }
                break;
            }
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                    printf("Current directory: %s\n", cwd);
                else perror("getcwd error");
                break;
            }
            case 'v':
                printf("--- Environment variables ---\n");
                for (char **env = environ; *env != NULL; env++) printf("%s\n", *env);
                printf("----------------------------\n");
                break;
            case 'V':
                if (putenv(arg) != 0) perror("putenv error");
                else printf("Environment variable set: %s\n", arg);
                break;
        }
    }
    return 0;
}
