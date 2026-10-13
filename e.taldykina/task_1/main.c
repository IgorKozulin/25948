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
} Command;

int main(int argc, char *argv[]) {
    Command *cmds = malloc(argc * sizeof(Command));
    if (cmds == NULL) {
        perror("malloc failed");
        return 1;
    }

    int count = 0;
    int c;
    char *optstring = "ispuU:cC:dvV:";

    while ((c = getopt(argc, argv, optstring)) != -1) {
        cmds[count].opt = c;
        cmds[count].arg = optarg;
        count++;
    }

    for (int i = count - 1; i >= 0; i--) {
        switch (cmds[i].opt) {
            case 'i': {
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
            }
            case 's': {
                if (setpgid(0, 0) == 0) {
                    printf("Process became a process group leader (PGID=%d)\n", getpgrp());
                } else {
                    perror("setpgid failed");
                }
                break;
            }
            case 'p': {
                printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
                break;
            }
            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("Current ulimit (RLIMIT_NOFILE): soft=%ld, hard=%ld\n", 
                           (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("getrlimit failed");
                }
                break;
            }
            case 'U': {
                char *endptr;
                errno = 0;
                long val = strtol(cmds[i].arg, &endptr, 10);
                
                if (errno != 0 || endptr == cmds[i].arg || val < 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid value for -U: %s\n", cmds[i].arg);
                } else {
                    struct rlimit rl;
                    rl.rlim_cur = val;
                    rl.rlim_max = val;
                    if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
                        printf("Ulimit changed to %ld\n", val);
                    } else {
                        perror("setrlimit failed");
                    }
                }
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("Core file size limit: soft=%ld, hard=%ld bytes\n", 
                           (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("getrlimit (core) failed");
                }
                break;
            }
            case 'C': {
                char *endptr;
                errno = 0;
                long val = strtol(cmds[i].arg, &endptr, 10);
                
                if (errno != 0 || endptr == cmds[i].arg || val < 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid value for -C: %s\n", cmds[i].arg);
                } else {
                    struct rlimit rl;
                    rl.rlim_cur = val;
                    rl.rlim_max = val;
                    if (setrlimit(RLIMIT_CORE, &rl) == 0) {
                        printf("Core file size limit changed to %ld bytes\n", val);
                    } else {
                        perror("setrlimit (core) failed");
                    }
                }
                break;
            }
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current working directory: %s\n", cwd);
                } else {
                    perror("getcwd failed");
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
                if (putenv(cmds[i].arg) == 0) {
                    printf("Environment variable set: %s\n", cmds[i].arg);
                } else {
                    perror("putenv failed");
                }
                break;
            }
            case '?':
                break;
        }
    }
    
    free(cmds);
    return 0;
}
