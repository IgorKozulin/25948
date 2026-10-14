#include <stdio.h>
#include <time.h>// time, localtime, tzset, struct tm, tzname
#include <stdlib.h> // setenv
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

static void my_time();
static void show_ids(const char *label);
static void f_open(const char *path);

static void my_time(){
    setenv("TZ", "Asia/Novosibirsk", 1);
    tzset();

    time_t now = time(NULL);
    struct tm *sp = localtime(&now);
    if (!sp) { 
        perror("localtime"); 
        return;
    }

    printf("%02d/%02d/%04d %02d:%02d %s\n",sp->tm_mday, sp->tm_mon + 1, sp->tm_year + 1900, sp->tm_hour, sp->tm_min, tzname[sp->tm_isdst > 0 ? 1 : 0]);
    return;

}

int main(void) {
    if  (getuid() == geteuid()){
         my_time();
    }
    else{
        printf("You cannot find out this information.\n");
        printf("apply setuid?\n");

    }
    char s1[4] = "yes";
    char s2[4];
    fgets(s2, sizeof(s2), stdin); 
    if (s2[3] == '\n'){
        s2[3] = '\0';
    }
    if (!strcmp(s1, s2)){
        if (setuid(getuid()) != 0) {
            perror("setuid");
        }
        my_time();
        
    }
    return 0;
}