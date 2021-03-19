#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "logging.h"

const char *pt;
FILE *file;

int openfile(char *p){
    pt = getenv("LOG_FILENAME");
    file = fopen(pt,p);
    return 0;
};

int closefile(){
    fclose(file);
    return 0;
};

int logging(){
    if (getpgid(0) == getpid()) openfile("w");
    else  openfile("a");
    fprintf(file, "pid: %d \n" , getpid());
    closefile();
    return 0;
};
