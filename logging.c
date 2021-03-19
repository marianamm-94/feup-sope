#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "logging.h"
#include <time.h>

const char *pt;
FILE *file;

int openfile(char* p){
    pt = getenv("LOG_FILENAME");
    file = fopen(pt,p);
    return 0;
};

int closefile(){
    fclose(file);
    return 0;
};

int logging(char* action, char* info){
    char * b = getenv("INITIAL");
    double beg = strtod(b,NULL);
    clock_t now = clock();
    double instant = (double)(now-beg)/ CLOCKS_PER_SEC;
    instant = instant *1000;
    openfile("a");
    fprintf(file, "%le ; %d ; %s ; %s \n" , instant, getpid(),action,info);
    closefile();
    return 0;
};

int loggingFile(char* action, char* path, int o, int n){
    char * b = getenv("INITIAL");
    double beg = strtod(b,NULL);
    clock_t now = clock();
    double instant = (double)(now-beg)/ CLOCKS_PER_SEC;
    instant = instant *1000;
    openfile("a");
    fprintf(file, "%le ; %d ; %s ; %s : %o : %o \n" , instant, getpid(),action,path,o,n);
    closefile();
    return 0;
};
