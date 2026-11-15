#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

static void usage(const char *prog);

/* Снять mandatory lock bit: убрать setgid, вернуть group-execute */
static int clear_mandatory_bit(int fd);

/* Включить mandatory lock bit: setgid + снять group-execute */
static int set_mandatory_bit(int fd);


int main(int argc, char *argv[]){ //

    int mandatory_flag = 0;
    const char *path = NULL;

    for (int i = 1; i < argc; i++){

        if (strcmp(argv[i],"-m") == 0){mandatory_flag = 1;}
        else if (path == NULL){path = argv[i];}
        else{usage(argv[0]);}
         
    }
    if (path == NULL) {usage(argv[0]);}




    
    int f_data = open(path, O_RDWR);
    if (f_data== -1) {perror("open"); exit(EXIT_FAILURE); }

    
    int mand_set = 0;
    if (mandatory_flag) {
        if (set_mandatory_bit(f_data) == 0){
            mand_set = 1;
            printf("[+] mandatory lock bit установлен (setgid, без group-x)\n");
        }
    }

    // заявка ядру
    struct flock fl; // встроеннная структура для обрщения к ядру
    memset(&fl, 0, sizeof fl); //оистить от мусора
    fl.l_type = F_WRLCK;    // блокировка на запись 
    fl.l_whence = SEEK_SET; // отсчет с начала
    fl.l_start = 0; // смещение 0
    fl.l_len = 0; // до конца файла 

    if (fcntl(f_data, F_SETLK, &fl) == -1){
        if (errno == EACCES || errno == EAGAIN) //errno --  глобальная переменная, зап. ядром при  неудочном вызове
            fprintf(stderr, "[-] файл уже заблокирован другим процессом\n");
        else
            perror("fcntl");
        if (mand_set) clear_mandatory_bit(f_data);
        close(f_data);
        exit(EXIT_FAILURE);
    }


    printf("[+] Файл захвачен (%s). Открыт в дескрипторе%d\n", mandatory_flag ? "mandatory" : "advisory", f_data);

    // запуск редактора
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "vi %s", path); //запись команды
    printf("[*] Запуск: %s\n", cmd);
    int rc = system(cmd);//запуск
    if (rc == -1) perror("system");

    
    fl.l_type = F_UNLCK;
    if (fcntl(f_data, F_SETLK, &fl) == -1)
        perror("fcntl unlock");

    if (mand_set) clear_mandatory_bit(f_data);
    close(f_data);

    printf("[+] Блокировка снята, файл закрыт\n");
    return 0;
}

static void usage(const char *prog){
    fprintf(stderr, "Usage: %s [-m] <file>\n", prog);
    exit(EXIT_FAILURE);
}


static int set_mandatory_bit(int fd){
    struct stat st;
        if (fstat(fd, &st) == -1){
        perror("fstat"); 
        return -1;}

    st.st_mode |= S_ISGID;  // setgid выставить 
    st.st_mode &= ~S_IXGRP;// group-x снять

        if (fchmod(fd, st.st_mode) == -1) {
        perror("fchmod");
        return -1;}
    return 0;
}



static int clear_mandatory_bit(int fd){
    struct stat st;
    if (fstat(fd, &st) == -1){
        perror("fstat"); 
        return -1;}

    st.st_mode &= ~S_ISGID;  // setgid снять     
    st.st_mode |= S_IXGRP;  // group-x вернуть   

    if (fchmod(fd, st.st_mode) == -1) {
        perror("fchmod");
        return -1;}
    return 0;
}

