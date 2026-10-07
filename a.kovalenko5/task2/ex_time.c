#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(){
	setenv("TZ","America/Los_Angeles",1);
	tzset();
	
	time_t now;
	time(&now);
	struct tm *sp = localtime(&now);
	printf("%02d/%02d/%04d %02d:%02d %s\n",
	 sp->tm_mon + 1,      //month 0 to 11
         sp->tm_mday,        //day of month 
         sp->tm_year + 1900,  //years
         sp->tm_hour,         // hours
         sp->tm_min,          // minutes
         tzname[sp->tm_isdst]); // PST или PDT

    return 0;
}
