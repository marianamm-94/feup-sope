#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "signals.h"
#include "logging.h"

volatile sig_atomic_t hold = 0, end = 0, ask = 0;

extern int nftot;
extern int nfmod;
extern char* dir;


int setParent(){
    if(signal(SIGINT, parentinthandler))
    	logging("SIGNAL_RECV","SIGINT");
    if(signal(SIGUSR1, SIG_IGN))
    	logging("SIGNAL_RECV","SIGUSR1");
    return 0;
};

int setChild(){
    if(signal(SIGINT, childinthandler))
    	logging("SIGNAL_RECV","SIGINT");
    if(signal(SIGCONT, childcontinue))
    	logging("SIGNAL_RECV","SIGCONT");
    if(signal(SIGUSR1, childend))
    	logging("SIGNAL_RECV","SIGUSR1");
    return 0;
};

void childinthandler(int signo){
    hold = 1;
    printf("%d ; %s ; %d ; %d\n", getpid(), dir, nftot, nfmod);
};

void parentinthandler(int signo){
    printf("%d ; %s ; %d ; %d\n", getpid(), dir, nftot, nfmod);
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
    loggingSignal("SIGNAL_SENT", "SIGUSR1", getpgid(0));    
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
            printf("End the program ?(y/n) \n");
            c = getchar();
            if(c == 'y'|| c=='Y') 
            	parentterminate();
            if(c == 'n'|| c=='N') {
                loggingSignal("SIGNAL_SENT", "SIGCONT", getpgid(0));
            	killpg(getpgid(0), SIGCONT);
            	}
        }while (c != 'y' && c != 'n'&& c != 'Y' && c != 'N');

    }
    else if (getpgid(0) != getpid() && end)
    {
        childterminate();
    }
    return 0;
    
};


