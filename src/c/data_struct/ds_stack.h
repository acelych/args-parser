#ifndef __DS_STACK_H
#define __DS_STACK_H

#include "ds_common.h"

/*------------------------------ STACK ------------------------------*/

typedef struct StackNode {
	void *data;
	struct StackNode* last;
} StackNode;

/**
* Stack Struct for C language
* @brief This stack is based on linked list, using StackNode
* @param curr Current node that on the top of the stack
* @param length Length of the stack, or numbers of nodes
* @param deepFree Set true to free data when popping nodes, false to simply remove nodes
*/
typedef struct Stack {
	StackNode *curr;
	unsigned long long length;
	Bool deepFree;
	
	Bool(*empty)(struct Stack*);
	Signal(*push)(struct Stack*, void*);
	Signal(*pop)(struct Stack*);
	void* (*top)(struct Stack*);
	Signal(*clear)(struct Stack*);
	void (*delete)(struct Stack*);
} Stack;

Stack *Stack_GetInstance();

#endif