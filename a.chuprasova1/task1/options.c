#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

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
            printf("Invalid option: %c\n", optopt);
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
                setpgid(0, 0);
                printf("Process group ID: %d\n", getpgrp());
                break;

            case 'p':
                printf("PID: %d\n", getpid());
                printf("PPID: %d\n", getppid());
                printf("PGID: %d\n", getpgrp());
                break;

           case 'u':

                getrlimit(RLIMIT_NOFILE, &limit);

                printf("Ulimit: %ld\n", limit.rlim_cur);
                break;

            case 'U':

                getrlimit(RLIMIT_NOFILE, &limit);

                limit.rlim_cur = atol(options[i].argument);

                setrlimit(RLIMIT_NOFILE, &limit);
                break;

            case 'c':

                getrlimit(RLIMIT_CORE, &limit);

                printf("Core limit: %ld\n", limit.rlim_cur);
                break;

            case 'C':
                getrlimit(RLIMIT_CORE, &limit);
                limit.rlim_cur = atol(options[i].argument);
                setrlimit(RLIMIT_CORE, &limit);
                break;

            case 'd':
                getcwd(cwd, sizeof(cwd));

                printf("Current directory: %s\n", cwd);
                break;

            case 'v':
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;

            case 'V':
                putenv(options[i].argument);
                break;

        }
    }

    return 0;
}
