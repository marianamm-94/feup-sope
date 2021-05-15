#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "queue.h"

char fifoname[256];
time_t start;
int nsec, bufsz=5,fd;


void logging(Message *message, char *oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n", time(NULL), message->rid, message->tskload, message->pid,
           message->tid, message->tskres, oper);
}

int processInput(int argc, char *argv[]) {
    if (argc != 4 || argc!=6)
        return -1;
    if (strcmp(argv[1], "-t") != 0)
        return -1;
    if (atoi(argv[2]) <= 0)
        return -1;
    else
        nsec = atoi(argv[2]);
    if(argc == 4)
        strncpy(fifoname, argv[3], sizeof(fifoname));
    else{
        if (strcmp(argv[3], "-l") != 0)
            return -1;
        if(atoi(argv[4]<=0))
            return -1;
        bufsz=atoi(argv[4]);
         strncpy(fifoname, argv[5], sizeof(fifoname));
    } 
    return 0;
}

int main(int argc, char *argv[]) {

    if (processInput(argc, argv)) {
        perror("Invalid arguments!");
        exit(-1);
    }
    start = time(NULL);
    if (mkfifo(fifoname, 0660) < 0) {
        perror("Error while creating public FIFO!");
        exit(-1);
    }
    fd = open(fifoname,O_RDONLY);
    if(fd==-1){
        perror("Error while opening public FIFO!");
        exit(-1);
    }
    int id = 0;
    pthread_t consumidor;
    pthread_create(&consumidor, NULL, funcConsumidor, NULL);
    pthread_t *threads = malloc(sizeof(pthread_t) * 10000);
    while ((time(NULL) - start) < nsec) {
        Message request;
        read(fd,request,sizeof(Message));
        pthread_t thread;
        pthread_create(&thread, NULL, funcProdutor, &request);
        threads[id] = thread;
        id++;
    }
}