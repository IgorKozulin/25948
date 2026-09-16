#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>

extern char **environ;

typedef struct {
    int opt;
    char *arg;
} ParsedOption;

static void handle_option(int opt, const char *arg) {
    switch (opt) {
        case 'i': {
            printf("UID: real=%ld, effective=%ld | GID: real=%ld, effective=%ld\n",
                   (long)getuid(), (long)geteuid(),
                   (long)getgid(), (long)getegid());
            break;
        }
        case 's': {
            if (setpgid(0, 0) == -1) {
                perror("setpgid failed");
            } else {
                printf("Process became group leader (PGID: %ld)\n", (long)getpgrp());
            }
            break;
        }
        case 'p': {
            printf("PID: %ld, PPID: %ld, PGID: %ld\n",
                   (long)getpid(), (long)getppid(), (long)getpgrp());
            break;
        }
        case 'u': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("getrlimit(RLIMIT_NOFILE) failed");
            } else {
                printf("ulimit (RLIMIT_NOFILE): soft=%ld, hard=%ld\n",
                       (long)rl.rlim_cur, (long)rl.rlim_max);
            }
            break;
        }
        case 'U': {
            char *endptr = NULL;
            errno = 0;
            long val = strtol(arg, &endptr, 10);
            if (errno != 0 || *endptr != '\0' || val <= 0) {
                fprintf(stderr, "Error: Invalid new ulimit value '%s'\n", arg);
                break;
            }

            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("getrlimit failed");
                break;
            }

            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("setrlimit(RLIMIT_NOFILE) failed");
            } else {
                printf("ulimit (RLIMIT_NOFILE) successfully set to %ld\n", val);
            }
            break;
        }
        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("getrlimit(RLIMIT_CORE) failed");
            } else {
                printf("Core file size limit: soft=%ld bytes, hard=%ld bytes\n",
                       (long)rl.rlim_cur, (long)rl.rlim_max);
            }
            break;
        }
        case 'C': {
            char *endptr = NULL;
            errno = 0;
            long val = strtol(arg, &endptr, 10);
            if (errno != 0 || *endptr != '\0' || val < 0) {
                fprintf(stderr, "Error: Invalid core size '%s'\n", arg);
                break;
            }

            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("getrlimit failed");
                break;
            }

            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("setrlimit(RLIMIT_CORE) failed");
            } else {
                printf("Core file size successfully set to %ld bytes\n", val);
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
            printf("=== Environment Variables ===\n");
            for (char **env = environ; *env != NULL; ++env) {
                printf("%s\n", *env);
            }
            break;
        }
        case 'V': {
            if (strchr(arg, '=') == NULL) {
                fprintf(stderr, "Error: -V argument must be in NAME=VALUE format\n");
                break;
            }
            char *env_copy = strdup(arg);
            if (!env_copy) {
                perror("strdup failed");
                break;
            }
            if (putenv(env_copy) != 0) {
                perror("putenv failed");
                free(env_copy);
            } else {
                printf("Environment variable set: %s\n", arg);
            }
            break;
        }
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:";
    int c;

    ParsedOption *opts = malloc((size_t)argc * sizeof(ParsedOption));
    if (!opts) {
        perror("malloc failed");
        return 1;
    }

    int opt_count = 0;

    while ((c = getopt(argc, argv, optstring)) != -1) {
        if (c == '?') {
            continue;
        }
        opts[opt_count].opt = c;
        opts[opt_count].arg = optarg ? optarg : NULL;
        opt_count++;
    }

    // Выполнение опций СПРАВА НАЛЕВО
    for (int i = opt_count - 1; i >= 0; --i) {
        handle_option(opts[i].opt, opts[i].arg);
    }

    free(opts);
    return 0;
}
