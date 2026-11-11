#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

// Функция для превращения числа (mode) в строку прав доступа (например, "drwxr-xr-x")
void format_permissions(mode_t mode, char *str) {
    // 1. Определяем тип файла
    if (S_ISDIR(mode))       str[0] = 'd';
    else if (S_ISREG(mode))  str[0] = '-';
    else if (S_ISLNK(mode))  str[0] = 'l';
    else                     str[0] = '?';

    // 2. Права владельца (Owner)
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';

    // 3. Права группы (Group)
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';

    // 4. Права остальных (Others)
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';

    // Завершающий ноль для строки
    str[10] = '\0';
}

int main(int argc, char *argv[]) {
    // Если аргументов нет, выводим ошибку
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    struct stat st;
    
    // Проходим по всем аргументам командной строки
    for (int i = 1; i < argc; i++) {
        // lstat получает информацию о файле. Если это ссылка, 
        // возвращает информацию о самой ссылке, а не о том, куда она ведет.
        if (lstat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue; // Пропускаем этот файл, переходим к следующему
        }

        // 1. Формируем строку прав
        char perms[11];
        format_permissions(st.st_mode, perms);

        // 2. Получаем имя владельца (UID -> Имя)
        struct passwd *pw = getpwuid(st.st_uid);
        char *owner_name = pw ? pw->pw_name : "?";

        // 3. Получаем имя группы (GID -> Имя)
        struct group *gr = getgrgid(st.st_gid);
        char *group_name = gr ? gr->gr_name : "?";

        // 4. Формируем дату модификации
        char time_buf[64];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&st.st_mtime);
        
        // Если файл старше 6 месяцев (примерно 15552000 секунд), показываем год.
        // Иначе показываем часы и минуты.
        if (now - st.st_mtime > 15552000) {
            strftime(time_buf, sizeof(time_buf), "%b %e  %Y", tm_info);
        } else {
            strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm_info);
        }

        // 5. Размер выводим ТОЛЬКО для обычных файлов. Для каталогов и устройств — пусто.
        char size_str[32];
        if (S_ISREG(st.st_mode)) {
            snprintf(size_str, sizeof(size_str), "%lld", (long long)st.st_size);
        } else {
            size_str[0] = '\0'; // Пустая строка
        }

        // 6. Извлекаем только имя файла из пути (например, из /etc/passwd делаем passwd)
        // ВАЖНО: basename может модифицировать строку, поэтому передаем копию.
        char *path_copy = strdup(argv[i]);
        char *filename = basename(path_copy);

        // 7. Печатаем результат с выравниванием (как таблица)
        // %s - права, %4ld - ссылки (ширина 4), %-8s - имя (ширина 8, левое выравнивание)
        // %8s - размер (ширина 8, правое выравнивание), %s - дата, %s - имя файла
        printf("%s %4ld %-8s %-8s %8s %s %s\n",
               perms,
               (long)st.st_nlink,
               owner_name,
               group_name,
               size_str,
               time_buf,
               filename);
               
        free(path_copy); // Освобождаем память
    }

    return 0;
}
