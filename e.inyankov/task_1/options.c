#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>

extern char **environ; // Глобальная переменная для доступа к среде окружения

// Структура для сохранения опции и её аргумента
typedef struct {
    int opt;
    char *arg;
} OptionRecord;

int main(int argc, char *argv[]) {
    char *options = "ispuU:cC:dvV:t:";
    int c;
    
    // Выделяем память под массив структур (максимум опций = argc)
    OptionRecord *records = malloc(argc * sizeof(OptionRecord));
    if (records == NULL) {
        perror("malloc");
        return 1;
    }
    int count = 0;

    // 1. Считываем и сохраняем все опции слева направо
    opterr = 0; // Отключаем стандартные ошибки getopt для обработки '?'
    while ((c = getopt(argc, argv, options)) != -1) {
        if (c == '?') {
            printf("Invalid option: -%c\n", optopt);
            continue; // Пропускаем неверные опции
        }
        
        records[count].opt = c;
        if (optarg != NULL) {
            records[count].arg = strdup(optarg); // Копируем аргумент
        } else {
            records[count].arg = NULL;
        }
        count++;
    }

    // 2. Обрабатываем опции в обратном порядке (справа налево)
    for (int i = count - 1; i >= 0; i--) {
        struct rlimit rl;
        long val;
        char cwd[PATH_MAX];

        switch (records[i].opt) {
            case 'i':
                printf("[-i] UID: %d, EUID: %d, GID: %d, EGID: %d\n", getuid(), geteuid(), getgid(), getegid());
                break;
                
            case 's':
                if (setpgid(0, 0) == 0) {
                    printf("[-s] Process is now the group leader (PGID = %d).\n", getpgrp());
                } else {
                    perror("[-s] setpgid failed");
                }
                break;
                
            case 'p':
                printf("[-p] PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
                break;
                
            case 'u':
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("[-u] ulimit (RLIMIT_NOFILE): soft=%ld, hard=%ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("[-u] getrlimit ulimit");
                }
                break;
                
            case 'U':
                val = atol(records[i].arg);
                if (val < 0) {
                    printf("[-U] Invalid ulimit value: %s\n", records[i].arg);
                } else {
                    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                        rl.rlim_cur = val;
                        if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
                            printf("[-U] ulimit (RLIMIT_NOFILE) changed to %ld\n", val);
                        } else {
                            perror("[-U] setrlimit ulimit failed");
                        }
                    }
                }
                break;
                
            case 'c':
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("[-c] Core file size: soft=%ld, hard=%ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
                } else {
                    perror("[-c] getrlimit core");
                }
                break;
                
            case 'C':
                val = atol(records[i].arg);
                if (val < 0) {
                    printf("[-C] Invalid core size value: %s\n", records[i].arg);
                } else {
                    if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                        rl.rlim_cur = val;
                        if (setrlimit(RLIMIT_CORE, &rl) == 0) {
                            printf("[-C] Core file size changed to %ld\n", val);
                        } else {
                            perror("[-C] setrlimit core failed");
                        }
                    }
                }
                break;
                
            case 'd':
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("[-d] Current working directory: %s\n", cwd);
                } else {
                    perror("[-d] getcwd failed");
                }
                break;
                
            case 'v':
                printf("[-v] Environment variables:\n");
                for (char **env = environ; *env != 0; env++) {
                    printf("  %s\n", *env);
                }
                break;
                
            case 'V':
                if (putenv(records[i].arg) == 0) {
                    printf("[-V] Environment variable added/modified: %s\n", records[i].arg);
                } else {
                    perror("[-V] putenv failed");
                }
                break;
	    case 't':
                if (records[i].arg != NULL) {
                    setenv("TZ", records[i].arg, 1);
                    tzset();

                    time_t now;
                    time(&now);
                    struct tm *sp = localtime(&now);

                    printf("[-t] Зона: %s | Время: %02d/%02d/%04d %02d:%02d %s\n", 
                        records[i].arg,
                        sp->tm_mon + 1,
                        sp->tm_mday,
                        sp->tm_year + 1900,
                        sp->tm_hour,
                        sp->tm_min,
                        tzname[sp->tm_isdst]);
                }
                else {
                    printf("[-t] Ошибка: не указан часовой пояс\n");
                }
                break;
        }
    }

    // 3. Освобождаем выделенную память
    for (int i = 0; i < count; i++) {
        // Важно: putenv становится частью окружения, память из-под нее чистить нельзя!
        if (records[i].arg != NULL && records[i].opt != 'V') {
            free(records[i].arg);
        }
    }
    free(records);

    return 0;
}
