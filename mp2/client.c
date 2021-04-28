#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int nsec, serverClosed = 0, fd, clientClosed = 0;
char fifoname[256];
time_t start;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


void logging(Message* message, char * oper)
{
	printf("%ld ; %d ; %d ; %d ; %ld ; %d ; %s \n", time(NULL), message->rid, message->tskload, message->pid, message->tid, message->tskres, oper);
}


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
		return NULL;
	}

	
	//send request
	if (write(fd, message, sizeof(Message)) < 0)
	{
			unlink(privateFifo);
			pthread_mutex_lock(&mut);
			serverClosed = 1;
			pthread_mutex_unlock(&mut);
			free(message);
			return NULL;
	}
	pthread_mutex_lock(&mut);
	logging(message, "IWANT");
	pthread_mutex_unlock(&mut);
	
	
	//open privateFIFO for reading
	int fdPrivate = open(privateFifo, O_RDONLY);
	if (fdPrivate == -1)
	{
		if(clientClosed)
		{
			unlink(privateFifo);
			pthread_mutex_lock(&mut);
			logging(message, "GAVUP");
			pthread_mutex_unlock(&mut);
			close(fdPrivate);
			free(message);
			return NULL;
		}
	}

	//reads message
	Message* receiveMessage = malloc(sizeof(Message));

	
	if (read(fdPrivate, receiveMessage, sizeof(Message)) < 0)
	{
		if(clientClosed)
		{
			unlink(privateFifo);
			pthread_mutex_lock(&mut);
			logging(message, "GAVUP");
			pthread_mutex_unlock(&mut);
			close(fdPrivate);
			free(message);
			free(receiveMessage);
			return NULL;
		}
	}
	if (receiveMessage->tskres == -1)
		{

			pthread_mutex_lock(&mut);
			logging(receiveMessage, "CLOSD");
			serverClosed = 1;
			pthread_mutex_unlock(&mut);
		}
	else	
		{	
			pthread_mutex_lock(&mut);
			logging(receiveMessage, "GOTRS");
			pthread_mutex_unlock(&mut);
		}
	unlink(privateFifo);
	close(fdPrivate);
	free(receiveMessage);
	free(message);
	return NULL;
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
	fd = open(fifoname, O_WRONLY);
	if (fd == -1 )
		exit(-1);
	start = time(NULL);
	int id = 0;
	pthread_t *threads= malloc(sizeof(pthread_t)*10000);
	while ((time(NULL)-start)<nsec && !serverClosed)
	{
		int *temp_id =  malloc(sizeof(int));
		*temp_id = id;
		pthread_t thread;
		pthread_create(&thread, NULL, funcThread, temp_id);
		//pthread_detach(thread);
		usleep(10*1000); //creates every 50 ms
		threads[id]=thread;
		id++;
	}
	while((time(NULL)-start)<nsec);
	close(fd);
	clientClosed = 1;
	for (size_t i = 0; i < id; i++)
	{
		pthread_join(threads[i],NULL);
	}
	return 0;

}
