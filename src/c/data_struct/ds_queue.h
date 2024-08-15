#ifndef __DS_QUEUE_H
#define __DS_QUEUE_H

#include "ds_common.h"

/*------------------------------ QUEUE ------------------------------*/

typedef struct QueueNode {
	void *data;
	struct QueueNode* next;
} QueueNode;

/**
* Queue Struct for C language
* @brief This Queue is based on linked list, using QueueNode
* @param begin First node of the Queue, prepared to dequeue.
* @param end Last node of the Queue.
* @param length Length of the Queue, or numbers of nodes
* @param deepFree Set true to free data when clearing, false to simply remove nodes
*/
typedef struct Queue {
	QueueNode *begin, *end;
	unsigned long long length;
	Bool deepFree;
	
	Bool(*empty)(struct Queue*);
	Signal(*enqueue)(struct Queue*, void*);
	void* (*dequeue)(struct Queue*);
	Signal(*clear)(struct Queue*);
	void (*delete)(struct Queue*);
} Queue;

Queue *Queue_GetInstance();

#endif