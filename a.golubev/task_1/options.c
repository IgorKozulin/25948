#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

/* Структура для хранения распознанных опций */
struct opt_rec {
    int  opt;       /* буква опции */
    char *arg;      /* аргумент (для U, C, V), иначе NULL */
};

int main(int argc, char *argv[])
{
    const char *optstring = "ispuU:cC:dvV:";
    struct opt_rec *list = NULL;
    int count = 0;
    int c, i;

    /* 1) Собираем все опции в массив (getopt идёт слева направо) */
    while ((c = getopt(argc, argv, optstring)) != -1) {
        struct opt_rec *tmp = realloc(list, sizeof(struct opt_rec) * (count + 1));
        if (!tmp) {
            perror("realloc");
            free(list);
            return 1;
        }
        list = tmp;
        list[count].opt = c;
        list[count].arg = optarg ? strdup(optarg) : NULL;
        if (optarg && !list[count].arg) {
            perror("strdup");
            return 1;
        }
        count++;
    }

    /* 2) Выполняем опции справа налево */
    for (i = count - 1; i >= 0; i--) {
        switch (list[i].opt) {

        case 'i': {
            /* Реальные и эффективные UID/GID */
            printf("-i: real UID=%d, effective UID=%d, real GID=%d, effective GID=%d\n",
                   (int)getuid(), (int)geteuid(),
                   (int)getgid(), (int)getegid());
            break;
        }

        case 's': {
            /* Стать лидером группы процессов */
            if (setpgid(0, 0) == -1) {
                perror("-s: setpgid");
            } else {
                printf("-s: процесс стал лидером группы, PGID=%d\n",
                       (int)getpgrp());
            }
            break;
        }

        case 'p': {
            printf("-p: PID=%d, PPID=%d, PGID=%d\n",
                   (int)getpid(), (int)getppid(), (int)getpgrp());
            break;
        }

        case 'u': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                printf("-u: ulimit (RLIMIT_NOFILE) soft=%lu hard=%lu\n",
                       (unsigned long)rl.rlim_cur,
                       (unsigned long)rl.rlim_max);
            } else {
                perror("-u: getrlimit");
            }
            break;
        }

        case 'U': {
            /* Изменить ulimit (RLIMIT_NOFILE) */
            char *end = NULL;
            errno = 0;
            long val = strtol(list[i].arg, &end, 10);
            if (errno || end == list[i].arg || *end != '\0' || val < 0) {
                fprintf(stderr, "-U: неверное значение '%s'\n", list[i].arg);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("-U: getrlimit");
                break;
            }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("-U: setrlimit");
            } else {
                printf("-U: ulimit изменён на %ld\n", val);
            }
            break;
        }

        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                printf("-c: core file size soft=%lu hard=%lu\n",
                       (unsigned long)rl.rlim_cur,
                       (unsigned long)rl.rlim_max);
            } else {
                perror("-c: getrlimit");
            }
            break;
        }

        case 'C': {
            char *end = NULL;
            errno = 0;
            long val = strtol(list[i].arg, &end, 10);
            if (errno || end == list[i].arg || *end != '\0' || val < 0) {
                fprintf(stderr, "-C: неверное значение '%s'\n", list[i].arg);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("-C: getrlimit");
                break;
            }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("-C: setrlimit");
            } else {
                printf("-C: core file size изменён на %ld\n", val);
            }
            break;
        }

        case 'd': {
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf)) != NULL) {
                printf("-d: cwd=%s\n", buf);
            } else {
                perror("-d: getcwd");
            }
            break;
        }

        case 'v': {
            /* Вывод переменных среды */
            printf("-v: переменные среды:\n");
            for (char **e = environ; *e; e++) {
                printf("    %s\n", *e);
            }
            break;
        }

        case 'V': {
            /* Установить/изменить переменную среды */
            if (putenv(list[i].arg) == 0) {
                printf("-V: установлено '%s'\n", list[i].arg);
            } else {
                perror("-V: putenv");
            }
            break;
        }

        case '?':
        default:
            /* getopt уже напечатал сообщение */
            if (optopt) {
                fprintf(stderr, "Неизвестная опция: -%c\n", optopt);
            }
            break;
        }
    }

    /* Освобождаем память */
    for (i = 0; i < count; i++) {
        free(list[i].arg);
    }
    free(list);
    return 0;
}
