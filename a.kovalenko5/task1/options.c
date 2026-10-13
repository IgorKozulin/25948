#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>

extern char **environ;

int main(int argc, char *argv[]) {
    char *options = "ispuU:cC:dvV:";
    int c;
    struct rlimit rl;
    char cwd[1024];

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
            case 'i':
                printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
            case 's':
                if (setpgid(0, 0) == -1) perror("setpgid");
                else printf("Process became group leader. PGID: %d\n", getpgrp());
                break;
            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
                break;
            case 'u':
                if (getrlimit(RLIMIT_FSIZE, &rl) == 0)
                    printf("ulimit (file size limit): %lu\n", (unsigned long)rl.rlim_cur);
                else perror("getrlimit (u)");
                break;
            case 'U':
                if (getrlimit(RLIMIT_FSIZE, &rl) == 0) {
                    rl.rlim_cur = strtol(optarg, NULL, 10);
                    if (setrlimit(RLIMIT_FSIZE, &rl) != 0) perror("setrlimit (U)");
                    else printf("ulimit changed to: %ld\n", strtol(optarg, NULL, 10));
                }
                break;
            case 'c':
                if (getrlimit(RLIMIT_CORE, &rl) == 0)
                    printf("Core file size limit: %lu\n", (unsigned long)rl.rlim_cur);
                else perror("getrlimit (c)");
                break;
            case 'C':
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    rl.rlim_cur = strtol(optarg, NULL, 10);
                    if (setrlimit(RLIMIT_CORE, &rl) != 0) perror("setrlimit (C)");
                    else printf("Core file size changed to: %ld\n", strtol(optarg, NULL, 10));
                }
                break;
            case 'd':
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                    printf("Current working directory: %s\n", cwd);
                else perror("getcwd");
                break;
            case 'v':
                for (char **env = environ; *env != NULL; env++)
                    printf("%s\n", *env);
                break;
            case 'V':
                if (putenv(optarg) != 0) perror("putenv");
                else printf("Environment variable set: %s\n", optarg);
                break;
            case '?':
                if (optopt == 'U' || optopt == 'C' || optopt == 'V')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (optopt != 0)
                    fprintf(stderr, "Unknown option -%c.\n", optopt);
                else
                    fprintf(stderr, "Unknown option: %s\n", argv[optind - 1]);
                break;
            default:
                break;
        }
    }

    if (optind < argc) {
        printf("\nNon-option arguments:\n");
        while (optind < argc)
            printf("  %s\n", argv[optind++]);
    }

    return 0;
}
