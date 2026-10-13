#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    int opt;
    char *arg;
} OptionEntry;

static int parse_limit_val(const char *str, rlim_t *out_val) {
    char *endptr;
    errno = 0;
    long long val = strtoll(str, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || endptr == str || val < 0) {
        return -1;
    }
    *out_val = (rlim_t)val;
    return 0;
}

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:";
    int opt;

    OptionEntry *entries = NULL;
    size_t count = 0;
    size_t capacity = 0;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        if (opt == '?') {
            fprintf(stderr, "Ошибка: Неизвестная опция или пропущен аргумент.\n");
            free(entries);
            return EXIT_FAILURE;
        }

        if (count >= capacity) {
            capacity = (capacity == 0) ? 8 : capacity * 2;
            OptionEntry *tmp = realloc(entries, capacity * sizeof(OptionEntry));
            if (!tmp) {
                perror("realloc");
                free(entries);
                return EXIT_FAILURE;
            }
            entries = tmp;
        }

        entries[count].opt = opt;
        entries[count].arg = optarg;
        count++;
    }

    for (ssize_t i = (ssize_t)count - 1; i >= 0; i--) {
        switch (entries[i].opt) {
            case 'i': {
                printf("[i] Real UID: %ld, Effective UID: %ld | Real GID: %ld, Effective GID: %ld\n",
                       (long)getuid(), (long)geteuid(), (long)getgid(), (long)getegid());
                break;
            }

            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("[-] setpgid failed");
                } else {
                    printf("[s] Процесс стал лидером группы. Новый PGID: %ld\n", (long)getpgrp());
                }
                break;
            }

            case 'p': {
                printf("[p] PID: %ld, Parent PID: %ld, PGID: %ld\n",
                       (long)getpid(), (long)getppid(), (long)getpgrp());
                break;
            }

            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                    perror("[-] getrlimit(RLIMIT_NOFILE)");
                } else {
                    printf("[u] ulimit (RLIMIT_NOFILE): soft = %lu, hard = %lu\n",
                           (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                }
                break;
            }

            case 'U': {
                rlim_t new_val;
                if (parse_limit_val(entries[i].arg, &new_val) != 0) {
                    fprintf(stderr, "[-] Ошибка: некорректное значение для -U: '%s'\n", entries[i].arg);
                    break;
                }

                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                    perror("[-] getrlimit(RLIMIT_NOFILE)");
                    break;
                }

                rl.rlim_cur = new_val;
                if (new_val > rl.rlim_max) {
                    rl.rlim_max = new_val;
                }

                if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
                    perror("[-] setrlimit(RLIMIT_NOFILE) failed");
                } else {
                    printf("[U] Лимит открытых дескрипторов успешно изменен на %lu\n", (unsigned long)new_val);
                }
                break;
            }

            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("[-] getrlimit(RLIMIT_CORE)");
                } else {
                    printf("[c] Максимальный размер core-файла (байт): soft = %lu, hard = %lu\n",
                           (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                }
                break;
            }

            case 'C': {
                rlim_t new_size;
                if (parse_limit_val(entries[i].arg, &new_size) != 0) {
                    fprintf(stderr, "[-] Ошибка: некорректное значение для -C: '%s'\n", entries[i].arg);
                    break;
                }

                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("[-] getrlimit(RLIMIT_CORE)");
                    break;
                }

                rl.rlim_cur = new_size;
                if (new_size > rl.rlim_max) {
                    rl.rlim_max = new_size;
                }

                if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("[-] setrlimit(RLIMIT_CORE) failed");
                } else {
                    printf("[C] Лимит размера core-файла успешно изменен на %lu байт\n", (unsigned long)new_size);
                }
                break;
            }

            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("[d] Текущая директория: %s\n", cwd);
                } else {
                    perror("[-] getcwd");
                }
                break;
            }

            case 'v': {
                printf("[v] --- Переменные среды процесса ---\n");
                for (char **env = environ; *env != NULL; env++) {
                    printf("    %s\n", *env);
                }
                break;
            }

            case 'V': {
                if (strchr(entries[i].arg, '=') == NULL) {
                    fprintf(stderr, "[-] Ошибка: аргумент опции -V должен иметь формат 'NAME=value'\n");
                    break;
                }
                char *env_str = strdup(entries[i].arg);
                if (!env_str) {
                    perror("strdup");
                    break;
                }
                if (putenv(env_str) != 0) {
                    perror("[-] putenv failed");
                    free(env_str);
                } else {
                    printf("[V] Среда обновлена: %s\n", entries[i].arg);
                }
                break;
            }

            default:
                break;
        }
    }

    free(entries);
    return EXIT_SUCCESS;
}
