#include <time.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "queue.h"
#include "lib.h"
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

char fifoname[256];
time_t start;
int nsec, bufsz=5,fd;
int serverClosed = 0;
sem_t * semaphore;
struct Queue* queue;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
int numThreads = 0;

void alrm(int signo)
{
	serverClosed = 1;
	unlink(fifoname);
	
}

void logging(Message *message, char *oper) {
    printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s\n", time(NULL), message->rid, message->tskload, getpid(),
           pthread_self(), message->tskres, oper);
}

void *funcProdutor(void *request) {
	pthread_mutex_lock(&mut);
	numThreads++;
	pthread_mutex_unlock(&mut);
	Message* message = malloc(sizeof(Message));
	message = (Message*) request;
	int res = task(message->tskload);
	if(serverClosed)
		{
		message->tskres = -1;
		}
	else{
	message->tskres = res;
	logging(message,"TSKEX");
	}
	sem_wait(semaphore);
	enqueue(queue, *message);
	pthread_mutex_lock(&mut);
	numThreads--;
	pthread_mutex_unlock(&mut);
	pthread_exit(NULL);
}


void *funcConsumidor(void *v){
	pthread_mutex_lock(&mut);
	numThreads++;
	pthread_mutex_unlock(&mut);
	while (1){
		if(isEmpty(queue)!=1)
		{
			Message* message = malloc(sizeof(Message));
			*message = dequeue(queue);
			char privateFifo[256];
    			sprintf(privateFifo, "/tmp/%d.%ld", message->pid, message->tid);
			int fdPrivate = open(privateFifo, O_WRONLY);
			if(fdPrivate < 0)
			{	
				logging(message,"FAILD");
				sem_post(semaphore);
				continue;
				}
			message->pid = getpid();
    			message->tid = pthread_self();
			if(write(fdPrivate, message, sizeof(Message))<0)
				{
				logging(message,"FAILD");
				sem_post(semaphore);
				continue;
				}
			if(message->tskres==-1)
				logging(message,"2LATE");
				
			else
				logging(message,"TSKDN");
			sem_post(semaphore);
			close(fdPrivate);
		}
		if(serverClosed && isEmpty(queue)&& numThreads<=1)
		{
			
			break;
			}
			
	
	}
	pthread_mutex_lock(&mut);
	numThreads--;
	pthread_mutex_unlock(&mut);
	pthread_exit(NULL);
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
    int id=0;
    int n;
    pthread_t consumidor;
    pthread_create(&consumidor, NULL, funcConsumidor, NULL);
    pthread_t *threads = malloc(sizeof(pthread_t) * 10000);
    while (1) {
        Message* request = malloc(sizeof(Message));
        if((n=read(fd,request,sizeof(Message)))<sizeof(Message))
        {	
        		break;
        }
	if(n==-1)
		break;
        pthread_t thread;
        logging(request,"RECVD");
        pthread_create(&thread, NULL, funcProdutor, request);
        usleep(500);
        threads[id] = thread;
	id++;
    }
    while(numThreads>0);
    free(queue);
    free(semaphore);
    return 0;


}

