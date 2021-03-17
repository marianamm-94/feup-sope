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
};

int closefile(){
    fclose(file);
};

int logging(){
    if (getpgid(0) == getpid()) openfile("w");
    else  openfile("a");
    fprintf(file, "pid: %d \n" , getpid());
    closefile();
};