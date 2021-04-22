#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "logging.h"
#include <sys/time.h>

const char *pt;
FILE *file;

int openfile(char* p){
    pt = getenv("LOG_FILENAME");
    if(pt==NULL)
    	return 1;
    file = fopen(pt,p);
    return 0;
};

int closefile(){
    fclose(file);
    return 0;
};

int logging(char* action, char* info){
    char * b = getenv("INITIAL1");
    long secStart = strtol(b,NULL,10);
    char * b1 = getenv("INITIAL2");
    long microStart = strtol(b1,NULL,10);
    struct timeval now;
    gettimeofday(&now,NULL);
    long diff1 = now.tv_sec - secStart;
    long diff2 = now.tv_usec - microStart;
    long instant = ((diff1)*1000+diff2/1000.0)+0.5;
    if(openfile("a"))
    	return 1;
    fprintf(file, "%ld ; %d ; %s ; %s \n" , instant, getpid(),action,info);
    closefile();
    return 0;
};

int loggingFile(char* action, char* path, int o, int n){
    char * b = getenv("INITIAL1");
    long secStart = strtol(b,NULL,10);
    char * b1 = getenv("INITIAL2");
    long microStart = strtol(b1,NULL,10);
    struct timeval now;
    gettimeofday(&now,NULL);
    long diff1 = now.tv_sec - secStart;
    long diff2 = now.tv_usec - microStart;
    long instant = ((diff1)*1000+diff2/1000.0)+0.5;
     if(openfile("a"))
    	return 1;
    fprintf(file, "%ld ; %d ; %s ; %s : %o : %o \n" , instant, getpid(),action,path,o,n);
    closefile();
    return 0;
};

int loggingSignal(char* action, char* signal, int pid){
    char * b = getenv("INITIAL1");
    long secStart = strtol(b,NULL,10);
    char * b1 = getenv("INITIAL2");
    long microStart = strtol(b1,NULL,10);
    struct timeval now;
    gettimeofday(&now,NULL);
    long diff1 = now.tv_sec - secStart;
    long diff2 = now.tv_usec - microStart;
    long instant = ((diff1)*1000+diff2/1000.0)+0.5;
    if(openfile("a"))
    	return 1;
    fprintf(file, "%ld ; %d ; %s ; %s : %d \n" , instant, getpid(),action,signal,pid);
    closefile();
    return 0;
};
