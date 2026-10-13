#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>          

extern char **environ;

struct Option {
        int option;
        char *argument;
    };

int main(int argc, char *argv[])
{
    struct Option options[argc];
    int count = 0;
    int c;
    struct rlimit limit;
    char cwd[1024];

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (c == '?') {
   
            if (optopt)
                printf("Invalid option: %c\n", optopt);
            else
                printf("Invalid option\n");
            continue;
        }
        options[count].option = c;
        options[count].argument = optarg;
        count++;
    }

    for(int i=count-1; i>=0; i--){
        switch (options[i].option) {

            case 'i':
                printf("Real UID: %d\n", getuid());
                printf("Effective UID: %d\n", geteuid());
                printf("Real GID: %d\n", getgid());
                printf("Effective GID: %d\n", getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                else
                    printf("Process group ID: %d\n", getpgrp());
                break;

            case 'p':
                printf("PID: %d\n", getpid());
                printf("PPID: %d\n", getppid());
                printf("PGID: %d\n", getpgrp());
                break;

           case 'u':
                getrlimit(RLIMIT_NOFILE, &limit);
                printf("Ulimit: %lu\n", (unsigned long)limit.rlim_cur);
                break;

            case 'U': {
                long v = atol(options[i].argument);
                if (v < 0) {
                    fprintf(stderr, "-U: invalid value '%s'\n",
                            options[i].argument);
                    break;
                }
                getrlimit(RLIMIT_NOFILE, &limit);
                limit.rlim_cur = v;
                if (setrlimit(RLIMIT_NOFILE, &limit) == -1)
                    perror("setrlimit");
                break;
            }

            case 'c':
                getrlimit(RLIMIT_CORE, &limit);
                printf("Core limit: %lu\n", (unsigned long)limit.rlim_cur);
                break;

            case 'C': {
                long v = atol(options[i].argument);
                if (v < 0) {
                    fprintf(stderr, "-C: invalid value '%s'\n",
                            options[i].argument);
                    break;
                }
                getrlimit(RLIMIT_CORE, &limit);
                limit.rlim_cur = v;
                if (setrlimit(RLIMIT_CORE, &limit) == -1)
                    perror("setrlimit");
                break;
            }

            case 'd':
                if (getcwd(cwd, sizeof(cwd)) == NULL)
                    perror("getcwd");
                else
                    printf("Current directory: %s\n", cwd);
                break;

            case 'v':
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;

            case 'V':
                if (!options[i].argument ||
                    !strchr(options[i].argument, '=')) {
                    fprintf(stderr, "-V: expected NAME=value\n");
                } else {
                    putenv(options[i].argument);
                }
                break;
        }
    }

    return 0;
}
