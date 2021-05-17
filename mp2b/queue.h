#ifndef QUEUE_H
#define QUEUE_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

struct Queue {
	int front, rear, size;
	unsigned capacity;
	Message* array;
};

struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, Message item);
Message dequeue(struct Queue* queue);
Message getFront(struct Queue* queue);
Message getRear(struct Queue* queue);

#endif 
