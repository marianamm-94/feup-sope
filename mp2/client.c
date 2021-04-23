#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

int nsec, serverClosed = 0;
char fifoname[256];
time_t start;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void *funcThread(void * id)
{
	int requestID = *(int*) id;

	Message *message = malloc(sizeof(Message));
	message->rid = requestID;
	message->pid = getpid();
	message->tid = pthread_self();
	message->tskload = rand() % 9 + 1;
	message->tskres = -1;

	//create private fifo
	char privateFifo[256];
	sprintf(privateFifo, "/tmp/%d.%ld", message->pid, message->tid);
	if (mkfifo(privateFifo, 0660) < 0)
	{
		perror("Error while creating private FIFO!");
		exit(-1);
	}
	
	//open public fifo
	int fd = open(fifoname, O_WRONLY);
	if (fd == -1 )
	{
		//log FAILD?
		unlink(privateFifo);
		pthread_mutex_lock(&mut);
		serverClosed = 1;
		pthread_mutex_unlock(&mut);
		exit(-1);
	}
	
	//send request
	if (write(fd, message, sizeof(Message)) < 0)
	{
		//log FAILD?
		unlink(privateFifo);
		pthread_mutex_lock(&mut);
		serverClosed = 1;
		pthread_mutex_unlock(&mut);
		exit(-1);
	}
	close(fd);
	//log IWANT
	
	//open privateFIFO for reading
	int fdPrivate = open(privateFifo, O_RDONLY);
	if (fdPrivate == -1)
	{
		//??
	}
	
	//reads message
	if (read(fdPrivate, message, sizeof(Message)) < 0)
	{
		//??
	}
	//CLOSD or GOTRS dependendo da mensagem recebida
	close(fdPrivate);
	free(message);
	exit(0);
}


int processInput(int argc, char *argv[])
{
	if(argc!=4)
		return -1;
	if(strcmp(argv[1], "-t")!=0)
		return -1;
	if(atoi(argv[2])<=0)
		return -1;
	else
		nsec = atoi(argv[2]);
	strncpy(fifoname, argv[3], sizeof(fifoname));
	
	return 0;
}
int main(int argc, char *argv[])
{

	if(processInput(argc, argv))
	{
		perror("Invalid arguments!");
		exit(-1);
	}

	start = time(NULL);
	int id = 1;
	while ((time(NULL)-start)<nsec)
	{
		pthread_t thread;
		pthread_create(thread, NULL, funcThread, &id);
		pthread_detach(thread);
		usleep(10*1000); //creates every 50 ms
		id++;
	}
	pthread_exit(0);

}
