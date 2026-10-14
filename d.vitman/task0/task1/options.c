#include <stdio.h>       
#include <stdlib.h>        
#include <unistd.h>         
#include <sys/resource.h>   
#include <sys/types.h>      
#include <limits.h>        
#include <string.h>         
#include <errno.h>         

extern char **environ;

#define MAX_OPTS 256

struct option_rec {
    char letter;        
    char *arg;        
};

void do_i(void) { //идентификаторы
    printf("-i\n");
    printf("Real UID:      %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
    printf("Real GID:      %d\n", (int)getgid());
    printf("Effective GID: %d\n", (int)getegid());
}

void do_s(void) { // изменение лидера группы
    printf("-s\n");
    
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    } else {
        printf("Process is now a group leader. PGID = %d\n", (int)getpgid(0));
    }
}

void do_p(void) {// идентификаторы процесса, родителя и группы
    printf("-p\n");
    printf("PID:  %d\n", (int)getpid());
    printf("PPID: %d\n", (int)getppid());
    printf("PGID: %d\n", (int)getpgid(0));
}


void do_u(void) { //ulimit
    struct rlimit rl;
    printf("-u\n");
    if (getrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("getrlimit(RLIMIT_FSIZE)");
        return;
    }
    printf("ulimit (RLIMIT_FSIZE): soft = %ld, hard = %ld\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("  (soft limit is infinity)\n");
}

// изменяет ulimit
void do_U(const char *arg) {
    struct rlimit rl;
    long newval;
    char *endptr;

    printf("-U %s\n", arg);

    // преобразуем строку в число
    errno = 0;
    newval = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || newval < 0) {
        fprintf(stderr, "Invalid value for -U: '%s'\n", arg);
        return;
    }

    if (getrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("getrlimit(RLIMIT_FSIZE)");
        return;
    }

    rl.rlim_cur = (rlim_t)newval;
    if (setrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("setrlimit(RLIMIT_FSIZE)");
        return;
    }
    printf("ulimit changed to %ld\n", newval);
}

void do_c(void) {// размер core-файла
    struct rlimit rl;
    printf("-c\n");
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit(RLIMIT_CORE)");
        return;
    }
    printf("core file size (RLIMIT_CORE): soft = %ld, hard = %ld\n",
           (long)rl.rlim_cur, (long)rl.rlim_max);
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("  (soft limit is infinity)\n");
}

void do_C(const char *arg) { // изменяет размер core-файла
    struct rlimit rl;
    long newval;
    char *endptr;

    printf("-C %s\n", arg);

    errno = 0;
    newval = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || newval < 0) {
        fprintf(stderr, "Invalid value for -C: '%s'\n", arg);
        return;
    }

    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit(RLIMIT_CORE)");
        return;
    }

    rl.rlim_cur = (rlim_t)newval;
    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit(RLIMIT_CORE)");
        return;
    }
    printf("core file size changed to %ld\n", newval);
}

void do_d(void) { //текущая директория
    char buf[PATH_MAX];
    printf("-d\n");
    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("getcwd");
        return;
    }
    printf("Current directory: %s\n", buf);
}

void do_v(void) { // переменные среды и их значения
    char **p;
    printf("-v\n");
    printf("Environment variables:\n");
    for (p = environ; *p != NULL; p++) {
        printf("  %s\n", *p);
    }
}

void do_V(const char *arg) { // изменение переменных сред
    printf("-V %s\n", arg);

    if (strchr(arg, '=') == NULL) {
        fprintf(stderr, "Invalid format for -V: '%s' (expected NAME=value)\n", arg);
        return;
    }

    char *copy = strdup(arg);
    if (copy == NULL) {
        perror("strdup");
        return;
    }

    if (putenv(copy) != 0) {
        perror("putenv");
        free(copy);
        return;
    }
    printf("Environment variable set: %s\n", arg);
}

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:";

    struct option_rec opts[MAX_OPTS];
    int nopts = 0;   
    int opt;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        if (nopts >= MAX_OPTS) {
            fprintf(stderr, "Too many options (max %d)\n", MAX_OPTS);
            return 1;
        }

        switch (opt) {
            case 'i': case 's': case 'p': case 'u':   // без аргументов
            case 'c': case 'd': case 'v':
                opts[nopts].letter = (char)opt;
                opts[nopts].arg = NULL;
                nopts++;
                break;

            case 'U': case 'C': case 'V':        // с аргументами
                opts[nopts].letter = (char)opt;
                opts[nopts].arg = optarg;
                nopts++;
                break;

            case '?':
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                break;

            default:
                fprintf(stderr, "Unexpected return from getopt: %d\n", opt);
                break;
        }
    }

    for (int i = nopts - 1; i >= 0; i--) {
        switch (opts[i].letter) {
            case 'i': do_i(); break;
            case 's': do_s(); break;
            case 'p': do_p(); break;
            case 'u': do_u(); break;
            case 'U': do_U(opts[i].arg); break;
            case 'c': do_c(); break;
            case 'C': do_C(opts[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V(opts[i].arg); break;
            default:
                fprintf(stderr, "Internal error: unknown option '%c'\n",
                        opts[i].letter);
                break;
        }
    }

    return 0;
}
