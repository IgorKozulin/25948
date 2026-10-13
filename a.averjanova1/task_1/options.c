#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    char option;
    char *argument;
} Option;

int main(int argc, char *argv[]) {
    Option *options = NULL;
    int count = 0;
    int c;

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (c == '?') {
            fprintf(stderr, "Invalid option: -%c\n", optopt);
            free(options);
            return 1;
        }

        Option *tmp = realloc(options, (count + 1) * sizeof(Option));

        if (tmp == NULL) {
            free(options);
            return 1;
        }

        options = tmp;
        options[count].option = (char)c;
        options[count].argument = optarg ? strdup(optarg) : NULL;
        count++;
    }

    for (int i = count - 1; i >= 0; i--) {
        switch (options[i].option) {
            case 'i':
                printf("UID: %ld, EUID: %ld\n",
                       (long)getuid(), (long)geteuid());
                printf("GID: %ld, EGID: %ld\n",
                       (long)getgid(), (long)getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                break;

            case 'p':
                printf("PID: %ld, PPID: %ld, PGID: %ld\n",
                       (long)getpid(),
                       (long)getppid(),
                       (long)getpgrp());
                break;

            case 'u': {
                struct rlimit limit;

                if (getrlimit(RLIMIT_NOFILE, &limit) == 0)
                    printf("ulimit: %lu\n",
                           (unsigned long)limit.rlim_cur);
                else
                    perror("getrlimit");

                break;
            }

            case 'U': {
                char *end;
                errno = 0;
                long value = strtol(options[i].argument, &end, 10);

                if (errno != 0 || *end != '\0' || value < 0) {
                    fprintf(stderr, "Invalid value for -U\n");
                    break;
                }

                struct rlimit limit;

                if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
                    perror("getrlimit");
                    break;
                }

                limit.rlim_cur = (rlim_t)value;

                if (setrlimit(RLIMIT_NOFILE, &limit) == -1)
                    perror("setrlimit");

                break;
            }

            case 'c': {
 		 struct rlimit limit;

   		 if (getrlimit(RLIMIT_CORE, &limit) == 0) {
       			 if (limit.rlim_cur == RLIM_INFINITY)
           			 printf("Core file size: unlimited\n");
       			 else
           			 printf("Core file size: %lu bytes\n",
                  			 (unsigned long)limit.rlim_cur);
   		 } else {
       			 perror("getrlimit");
   		 }

   		 break;
	    }

            case 'C': {
                char *end;
                errno = 0;
                long value = strtol(options[i].argument, &end, 10);

                if (errno != 0 || *end != '\0' || value < 0) {
                    fprintf(stderr, "Invalid value for -C\n");
                    break;
                }

                struct rlimit limit;

                if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                    perror("getrlimit");
                    break;
                }

                limit.rlim_cur = (rlim_t)value;

                if (setrlimit(RLIMIT_CORE, &limit) == -1)
                    perror("setrlimit");

                break;
            }

            case 'd': {
                char cwd[PATH_MAX];

                if (getcwd(cwd, sizeof(cwd)) != NULL)
                    printf("%s\n", cwd);
                else
                    perror("getcwd");

                break;
            }

            case 'v':
                for (char **env = environ; *env != NULL; env++)
                    printf("%s\n", *env);
                break;

            case 'V':
                if (putenv(options[i].argument) != 0) {
                    perror("putenv");
                } else {
                    options[i].argument = NULL;
                }
                break;
        }
    }

    for (int i = 0; i < count; i++)
        free(options[i].argument);

    free(options);
    return 0;
}

