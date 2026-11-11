#include <stdio.h> 
#include <stdlib.h> // strtol, putenv
#include <unistd.h> // getopt, getuid, geteuid, getgid, getegid, getpid, getppid, getpgid, setpgid, getcwd
#include <sys/resource.h> // struct rlimit, getrlimit, setrlimit, RLIMIT_NOFILE, RLIMIT_CORE
#include <limits.h> // PATH_MAX
#include <string.h> // strlen, strcpy
#include <errno.h> // errno

/*
•	-i  Печатает реальные и эффективные идентификаторы пользователя и группы.
•	-s  Процесс становится лидером группы. Подсказка: смотри setpgid(2).
•	-p  Печатает идентификаторы процесса, процесса-родителя и группы процессов.
•	-u  Печатает значение ulimit
•	-Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)
•	-c  Печатает размер в байтах core-файла, который может быть создан.
•	-Csize  Изменяет размер core-файла
•	-d  Печатает текущую рабочую директорию
•	-v  Распечатывает переменные среды и их значения
•	-Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной.
*/

extern char **environ;

typedef struct { 
    char opt; 
    char *arg; 
} OptEntry;

static OptEntry opts[100];
static int opt_count = 0;

static void option(char opt, char *arg);

int main(int argc, char *argv[]){
    int c;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1){
        if (c == '?') {
            fprintf(stderr, "Invalid option: %c\n", optopt); 
            continue; 
        }
        opts[opt_count].opt = c;
        opts[opt_count].arg = optarg;
        opt_count++;
    }
    for (int i = opt_count - 1; i >= 0; i--)
        option(opts[i].opt, opts[i].arg);
    return 0;
}

static void option(char opt, char *arg) {
    switch (opt) {
        case 'i':
            printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid()); 
            printf("Real GID: %d, Effective GID: %d\n", getgid(), getegid());
            break;
        case 's':
            if (setpgid(0, 0) == 0)
                printf("Process is now a group leader. PGID: %d\n", getpgid(0));
            else perror("setpgid");
            break;
        case 'p':
            printf("PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgid(0));
            break;
        case 'u': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
                printf("ulimit (RLIMIT_NOFILE): soft=%ld, hard=%ld\n",
                       (long)rl.rlim_cur, (long)rl.rlim_max);
            else perror("getrlimit");
            break;
        }
        case 'U': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                char *end; errno = 0;
                long v = strtol(arg, &end, 10);
                if (errno || *end || v < 0) {
                    fprintf(stderr, "Invalid value for -U: %s\n", arg);
                    break;
                }
                rl.rlim_cur = v;
                if (setrlimit(RLIMIT_NOFILE, &rl) == 0)
                    printf("ulimit changed to: %ld\n", v);
                else perror("setrlimit");
            }
            break;
        }
        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0)
                printf("Core file size: soft=%ld, hard=%ld\n",
                       (long)rl.rlim_cur, (long)rl.rlim_max);
            else perror("getrlimit");
            break;
        }
        case 'C': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0){
                char *end; errno = 0;
                long v = strtol(arg, &end, 10);
                if (errno || *end || v < 0) {
                    fprintf(stderr, "Invalid value for -C: %s\n", arg);
                    break;
                }
                rl.rlim_cur = v;
                if (setrlimit(RLIMIT_CORE, &rl) == 0)
                    printf("Core file size changed to: %ld\n", v);
                else perror("setrlimit");
            }
            break;
        }
        case 'd': {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd))) printf("Current directory: %s\n", cwd);
            else perror("getcwd");
            break;
        }
        case 'v':
            for (char **e = environ; *e; e++) printf("%s\n", *e);
            break;
        case 'V':
            if (putenv(arg) == 0) printf("Environment variable set: %s\n", arg);
            else perror("putenv");
            break;
    }
}

