#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "queue.h"
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

char fifoname[256];
time_t start;
int nsec, bufsz=5,fd;
int serverClosed = 0;
sem_t * semaphore;
struct Queue* queue;

void alrm(int signo)
{
	serverClosed = 1;
}

void logging(Message *message, char *oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n", time(NULL), message->rid, message->tskload, getpid(),
           pthread_self(), message->tskres, oper);
}

void *funcProdutor(void *request) {
	Message* message = malloc(sizeof(Message));
	message = (Message*) request;
	int res = task(message->tskload);
	if(serverClosed)
		{
		message->tskres = -1;
		logging(message,"2LATE");
		}
	else{
	message->tskres = res;
	logging(message,"TSKEK");
	}
	sem_wait(semaphore);
	enqueue(queue, *message);
	free(message);
}

void *funcConsumidor(void *v){
	while (!serverClosed){
		if(isEmpty(queue)!=1)
		{
			Message* message = malloc(sizeof(Message));
			*message = dequeue(queue);
			sem_post(semaphore);
			char privateFifo[256];
    			sprintf(privateFifo, "/tmp/%d.%ld", message->pid, message->tid);
			int fdPrivate = open(privateFifo, O_WRONLY);
			if(fdPrivate == -1)
			{	

				logging(message,"FAILD");
				free(message);
				break;
				}
			message->pid = getpid();
    			message->tid = pthread_self();
			if(write(fdPrivate, message, sizeof(Message))<0)
				{
				logging(message,"FAILD");
				free(message);
				break;
				}
			logging(message,"TSKDN");
			free(message);
		}
	
	
	
	}
}

int processInput(int argc, char *argv[]) {
    if (argc != 4 && argc!=6)
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
        if(atoi(argv[4])<=0)
            return -1;
        bufsz=atoi(argv[4]);
        strncpy(fifoname, argv[5], sizeof(fifoname));
    } 
    return 0;
}

int main(int argc, char *argv[]) {

    if (processInput(argc, argv)) {
        perror("Invalid arguments!");
        exit(1);
    }
    semaphore = (sem_t*)malloc(sizeof(sem_t));
    sem_init(semaphore,1,bufsz);
    queue = createQueue(bufsz);
    struct sigaction new,old;
    sigset_t smask;
    sigemptyset(&smask);
    new.sa_handler = alrm;
    new.sa_mask = smask;
    new.sa_flags= 0;
    if(sigaction(SIGALRM, &new, &old)==-1){
    	perror("sigaction SIGALRM error");
    	exit(2);
    }
    alarm(nsec);
    if (mkfifo(fifoname, 0660) < 0) {
        perror("Error while creating public FIFO!");
        exit(3);
    }
    fd = open(fifoname,O_RDONLY);
    if(fd==-1){
        perror("Error while opening public FIFO!");
        exit(3);
    }
    int id = 0;
    pthread_t consumidor;
    pthread_create(&consumidor, NULL, funcConsumidor, NULL);
    pthread_t *threads = malloc(sizeof(pthread_t) * 10000);
    while (!serverClosed) {
        Message* request = malloc(sizeof(Message));
        read(fd,request,sizeof(Message));
        logging(request,"RECVD");
        pthread_t thread;
        pthread_create(&thread, NULL, funcProdutor, request);
        threads[id] = thread;
	id++;
    }
    pthread_join(consumidor, NULL);
    for (size_t i = 0; i < id; i++) {
        pthread_join(threads[i], NULL);
    }
    free(queue);
    free(semaphore);
    close(fd);
    return 0;

}
