#include "ds_queue.h"

/**
* Return if the Queue is empty
* @param this Queue itself
* @return Bool: Queue is empty
*/
Bool empty_queue(Queue *this) {
	return this->begin == NULL;
}

/**
* Add a new node to the end
* @param this Queue itself
* @param data Data that the new node is pointing at
* @return Status
*/
Signal enqueue(Queue *this, void *data)
{
	if (this->empty(this))                                // Empty Queue
    {
		QueueNode *node = malloc(sizeof(QueueNode));
		node->data = data;
		node->next = NULL;
		this->begin = node;
        this->end = node;
		this->length = 1;
	}
	else                                                  // Has Something
    {
		QueueNode *node = malloc(sizeof(QueueNode));
		node->data = data;
		node->next = NULL;
        this->end->next = node;
        this->end = node;
		this->length++;
	}
	return Success;
}

/**
* Remove first node
* @param this Queue itself
* @return Status, if Queue is already empty, return NULL
*/
void* dequeue(Queue *this)
{
    if (this->empty(this)) return NULL;
    else
    {
        void *out = this->begin->data;
        QueueNode *out_node = this->begin;
        this->begin = out_node->next;
        if (this->end == out_node) this->end = NULL;
        this->length--;
        free(out_node);
        return out;
    }
}

Signal clear_queue(Queue *this) {
	while (!this->empty(this))
    {
		void *data = this->dequeue(this);
        if (this->deepFree) free(data);
	}
	return Success;
}

void delete_queue(Queue *this)
{
	this->clear(this);
	free(this);
}

Queue* Queue_GetInstance()
{
    Queue *instance = malloc(sizeof(Queue));
    instance->begin = NULL;
    instance->end = NULL;
    instance->length = 0;
    instance->deepFree = false;

    instance->empty = empty_queue;
    instance->enqueue = enqueue;
    instance->dequeue = dequeue;
    instance->clear = clear_queue;
    instance->delete = delete_queue;

    return instance;
}