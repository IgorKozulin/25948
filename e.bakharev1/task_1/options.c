#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

extern char **environ;

typedef enum { OP_I, OP_S, OP_P, OP_U, OP_U_VAL, OP_C, OP_C_VAL, OP_D, OP_V, OP_V_VAL, OP_T } OpType;

typedef struct {
    OpType type;
    char *arg;
} Action;

void execute_action(Action action) {
    struct rlimit rl;
    char cwd[PATH_MAX];

    switch (action.type) {
        case OP_I:
            printf("Real UID: %d, Effective UID: %d, Real GID: %d, Effective GID: %d\n",
                   getuid(), geteuid(), getgid(), getegid());
            break;
        case OP_S:
            if (setpgid(0, 0) == -1) perror("setpgid failed");
            else printf("Process became process group leader (PGID=%d)\n", getpgrp());
            break;
        case OP_P:
            printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
            break;
        case OP_U:
            getrlimit(RLIMIT_NOFILE, &rl);
            printf("Current ulimit (NOFILE): %ld\n", (long)rl.rlim_cur);
            break;
        case OP_U_VAL: {
            long val = strtol(action.arg, NULL, 10);
            getrlimit(RLIMIT_NOFILE, &rl);
            rl.rlim_cur = val;
            if (setrlimit(RLIMIT_NOFILE, &rl) == -1) perror("setrlimit (U) failed");
            else printf("Ulimit changed to: %ld\n", val);
            break;
        }
        case OP_C:
            getrlimit(RLIMIT_CORE, &rl);
            printf("Current core file size limit: %ld\n", (long)rl.rlim_cur);
            break;
        case OP_C_VAL: {
            long val = strtol(action.arg, NULL, 10);
            rl.rlim_cur = val;
            rl.rlim_max = val;
            if (setrlimit(RLIMIT_CORE, &rl) == -1) perror("setrlimit (C) failed");
            else printf("Core file size limit changed to: %ld\n", val);
            break;
        }
        case OP_D:
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("Current working directory: %s\n", cwd);
            else
                perror("getcwd failed");
            break;
        case OP_V:
            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }
            break;
        case OP_V_VAL:
            if (putenv(action.arg) != 0) perror("putenv failed");
            else printf("Environment variable set: %s\n", action.arg);
            break;
        case OP_T: {
            setenv("TZ", "Asia/Novosibirsk", 1);
            tzset();
            time_t now;
            time(&now);
            struct tm *sp = localtime(&now);
            printf("%02d/%02d/%04d %02d:%02d %s\n",
                   sp->tm_mday,
                   sp->tm_mon + 1,
                   sp->tm_year + 1900,
                   sp->tm_hour,
                   sp->tm_min,
                   tzname[sp->tm_isdst]);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int c;
    Action actions[argc];
    int action_count = 0;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:t")) != -1) {
        switch (c) {
            case 'i': actions[action_count++].type = OP_I; break;
            case 's': actions[action_count++].type = OP_S; break;
            case 'p': actions[action_count++].type = OP_P; break;
            case 'u': actions[action_count++].type = OP_U; break;
            case 'U': actions[action_count].type = OP_U_VAL; actions[action_count++].arg = optarg; break;
            case 'c': actions[action_count++].type = OP_C; break;
            case 'C': actions[action_count].type = OP_C_VAL; actions[action_count++].arg = optarg; break;
            case 'd': actions[action_count++].type = OP_D; break;
            case 'v': actions[action_count++].type = OP_V; break;
            case 'V': actions[action_count].type = OP_V_VAL; actions[action_count++].arg = optarg; break;
            case 't': actions[action_count++].type = OP_T; break;
            case '?':
                fprintf(stderr, "Недопустимая опция: %c\n", optopt);
                return 1;
        }
    }

    for (int i = action_count - 1; i >= 0; i--) {
        execute_action(actions[i]);
    }

    return 0;
}
