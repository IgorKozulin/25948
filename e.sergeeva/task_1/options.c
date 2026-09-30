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
} Option;

int main(int argc, char *argv[]) {
    Option options[100];
    int opt_count = 0;
    int c;
    char *optstring = "ispuU:cC:dvV:";

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
            case 'i':
            case 's':
            case 'p':
            case 'u':
            case 'c':
            case 'd':
            case 'v':
                options[opt_count].opt = c;
                options[opt_count].arg = NULL;
                opt_count++;
                break;
            case 'U':
            case 'C':
            case 'V':
                options[opt_count].opt = c;
                options[opt_count].arg = optarg;
                opt_count++;
                break;
            case '?':
                fprintf(stderr, "Invalid option: %c\n", optopt);
                return 1;
        }
    }

    for (int i = opt_count - 1; i >= 0; i--) {
        switch (options[i].opt) {
            case 'i': {
                printf("UID: real=%d, effective=%d\n", getuid(), geteuid());
                printf("GID: real=%d, effective=%d\n", getgid(), getegid());
                break;
            }
            case 's': {
                if (setpgid(0, 0) == 0) {
                    printf("Process became group leader (PGID=%d)\n", getpid());
                } else {
                    perror("setpgid");
                }
                break;
            }
            case 'p': {
                printf("PID=%d\n", getpid());
                printf("PPID=%d\n", getppid());
                printf("PGID=%d\n", getpgrp());
                break;
            }
            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("ulimit (open files): soft=%ld, hard=%ld\n", 
                           (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("getrlimit");
                }
                break;
            }
            case 'U': {
                long new_limit = atol(options[i].arg);
                struct rlimit rl;
                rl.rlim_cur = new_limit;
                rl.rlim_max = new_limit;
                if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("ulimit changed to %ld\n", new_limit);
                } else {
                    perror("setrlimit");
                }
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("Core file size: soft=%ld, hard=%ld bytes\n", 
                           (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("getrlimit");
                }
                break;
            }
            case 'C': {
                long new_size = atol(options[i].arg);
                struct rlimit rl;
                rl.rlim_cur = new_size;
                rl.rlim_max = new_size;
                if (setrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("Core file size changed to %ld bytes\n", new_size);
                } else {
                    perror("setrlimit");
                }
                break;
            }
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current directory: %s\n", cwd);
                } else {
                    perror("getcwd");
                }
                break;
            }
            case 'v': {
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;
            }
            case 'V': {
                if (putenv(options[i].arg) == 0) {
                    printf("Environment variable set: %s\n", options[i].arg);
                } else {
                    perror("putenv");
                }
                break;
            }
        }
    }

    return 0;
}
