#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "signals.h"

volatile sig_atomic_t hold = 0, end = 0, ask = 0;



int setParent(){
    signal(SIGINT, parentinthandler);
    signal(SIGUSR1, SIG_IGN);
    return 0;
};

int setChild(){
    signal(SIGINT, childinthandler);
    signal(SIGCONT, childcontinue);
    signal(SIGUSR1, childend);
    return 0;
};

void childinthandler(int signo){
    hold = 1;
    printf("child\n");
};

void parentinthandler(int signo){
     printf("parent\n");
    if(ask) return;
    ask = 1;
};

void childcontinue(int signo){
    hold = 0;
    end = 0;
};

void childend(int signo){
    hold = 1;
    end = 1;
};

void parentterminate(){
    usleep(10);
    killpg(getpgid(0), SIGUSR1);
    exit(1);
};

void childterminate(){
    while (wait(NULL) >= 0);
    return;
    
};

int checkcurrentstat(){
    if (getpgid(0) == getpid() && ask)
    {
        char c = 0;
        ask = 0;
        do {
            printf("End the program ? \n");
            c = getchar();
            getchar();
            if(c == 'y') parentterminate();
            if(c == 'n') killpg(getpgid(0), SIGCONT);
        }while (c != 'y' && c != 'n');

    }
    else if (getpgid(0) != getpid() && end)
    {
        childterminate();
    }
    return 0;
    
};


