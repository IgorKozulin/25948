#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/resource.h>

extern char **environ;

typedef struct {
    int opt;
    char *arg;
} ParsedOption;

void execute_option(int opt, const char *arg) {
    switch (opt) {
        case 'i': {
            printf("[-i] UID: real = %d, effective = %d; GID: real = %d, effective = %d\n",
                   (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());
            break;
        }
        case 's': {
            if (setpgid(0, 0) == -1) {
                perror("[-s] setpgid error");
            } else {
                printf("[-s] Процесс стал лидером группы. Новый PGID = %d\n", (int)getpgid(0));
            }
            break;
        }
        case 'p': {
            printf("[-p] PID = %d, PPID = %d, PGID = %d\n",
                   (int)getpid(), (int)getppid(), (int)getpgid(0));
            break;
        }
        case 'u': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                perror("[-u] getrlimit error");
            } else {
                printf("[-u] ulimit (RLIMIT_NOFILE): soft = %lu, hard = %lu\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
            }
            break;
        }
        case 'U': {
            char *endptr;
            errno = 0;
            long val = strtol(arg, &endptr, 10);
            if (*endptr != '\0' || val < 0 || errno != 0) {
                fprintf(stderr, "[-U] Ошибка: некорректное значение ulimit '%s'\n", arg);
            } else {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                    perror("[-U] getrlimit error");
                } else {
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
                        perror("[-U] setrlimit error");
                    } else {
                        printf("[-U] ulimit (RLIMIT_NOFILE) успешно изменён на %lu\n",
                               (unsigned long)rl.rlim_cur);
                    }
                }
            }
            break;
        }
        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                perror("[-c] getrlimit error");
            } else {
                printf("[-c] Максимальный размер core-файла: soft = %lu байт, hard = %lu байт\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
            }
            break;
        }
        case 'C': {
            char *endptr;
            errno = 0;
            long val = strtol(arg, &endptr, 10);
            if (*endptr != '\0' || val < 0 || errno != 0) {
                fprintf(stderr, "[-C] Ошибка: некорректный размер core-файла '%s'\n", arg);
            } else {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("[-C] getrlimit error");
                } else {
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("[-C] setrlimit error");
                    } else {
                        printf("[-C] Лимит core-файла успешно изменён на %lu байт\n",
                               (unsigned long)rl.rlim_cur);
                    }
                }
            }
            break;
        }
        case 'd': {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("[-d] Текущая рабочая директория: %s\n", cwd);
            } else {
                perror("[-d] getcwd error");
            }
            break;
        }
        case 'v': {
            printf("[-v] Переменные окружения:\n");
            for (char **env = environ; *env != NULL; env++) {
                printf("     %s\n", *env);
            }
            break;
        }
        case 'V': {
            if (strchr(arg, '=') == NULL) {
                fprintf(stderr, "[-V] Ошибка: формат переменной должен быть NAME=VALUE ('%s')\n", arg);
            } else {
                char *env_copy = strdup(arg);
                if (env_copy == NULL || putenv(env_copy) != 0) {
                    perror("[-V] putenv error");
                } else {
                    printf("[-V] Переменная окружения установлена: %s\n", arg);
                }
            }
            break;
        }
        case 't': {
            setenv("TZ", "Asia/Novosibirsk", 1);
            tzset();
            time_t now = time(NULL);
            struct tm *sp = localtime(&now);
            if (sp != NULL) {
                printf("[-t] Время в Новосибирске: %02d/%02d/%04d %02d:%02d:%02d %s\n",
                       sp->tm_mday,
                       sp->tm_mon + 1,
                       sp->tm_year + 1900,
                       sp->tm_hour,
                       sp->tm_min,
                       sp->tm_sec,
                       tzname[sp->tm_isdst]);
            } else {
                perror("[-t] localtime error");
            }
            break;
        }
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:t";
    int c;

    int capacity = 16;
    int count = 0;
    ParsedOption *opts = malloc(capacity * sizeof(ParsedOption));
    if (opts == NULL) {
        perror("malloc error");
        return EXIT_FAILURE;
    }

    while ((c = getopt(argc, argv, optstring)) != -1) {
        if (c == '?') {
            continue;
        }

        if (count >= capacity) {
            capacity *= 2;
            ParsedOption *temp = realloc(opts, capacity * sizeof(ParsedOption));
            if (temp == NULL) {
                perror("realloc error");
                free(opts);
                return EXIT_FAILURE;
            }
            opts = temp;
        }

        opts[count].opt = c;
        opts[count].arg = (optarg != NULL) ? strdup(optarg) : NULL;
        count++;
    }

    for (int i = count - 1; i >= 0; i--) {
        execute_option(opts[i].opt, opts[i].arg);
        if (opts[i].arg != NULL) {
            free(opts[i].arg);
        }
    }

    free(opts);
    return EXIT_SUCCESS;
}
