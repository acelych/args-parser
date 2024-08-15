#include "ds_stack.h"

/**
* Return if the stack is empty
* @param this Stack itself
* @return Bool: stack is empty
*/
Bool empty(Stack *this) {
	return this->curr == NULL;
}

/**
* Add a new node to the top
* @param this Stack itself
* @param data Data that the new node is pointing at
* @return Status
*/
Signal push(Stack *this, void *data) {
	if (this->empty(this)) {                                // Empty Stack
		StackNode *node = malloc(sizeof(StackNode));
		node->data = data;
		node->last = NULL;
		this->curr = node;
		this->length = 1;
	}
	else {                                                	// Has Something
		StackNode *node = malloc(sizeof(StackNode));
		node->data = data;
		node->last = this->curr;
		this->curr = node;
		this->length++;
	}
	return Success;
}


/**
* Remove top node
* @param this Stack itself
* @return Status, if stack is already empty, return Invalid
*/
Signal pop(Stack *this) {
	if (this->empty(this)) {
		return Invalid;
	}
	else {
		StackNode *last = this->curr->last;                 // Get Last
		if (this->deepFree)
			free(this->curr->data);                			// Release Data
		free(this->curr);                          			// Release Curr
		if (last != NULL) {
			this->curr = last;                              // Point to Last
			this->length--;
		}
		else {
			this->curr = NULL;                              // Empty
			this->length = 0;
		}
		return Success;
	}
}

/**
* Return the address of top node
* @param this Stack itself
* @return Address of node's data
*/
void *top(Stack *this) {
	if (this->empty(this)) return NULL;
	return this->curr->data;
}

Signal clear_stack(Stack *this) {
	while (!this->empty(this)) {
		this->pop(this);
	}
	return Success;
}

void delete_stack(Stack *this) {
	this->clear(this);
	free(this);
}

Stack *Stack_GetInstance() {
	Stack *instance = malloc(sizeof(Stack));
	instance->curr = NULL;
	instance->length = 0;
	instance->deepFree = false;

	instance->empty = &empty;
	instance->push = &push;
	instance->pop = &pop;
	instance->top = &top;
	instance->clear = &clear_stack;
	instance->delete = &delete_stack;

	return instance;
}