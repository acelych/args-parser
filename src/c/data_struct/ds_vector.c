#include "ds_vector.h"

#define MINIMUM_VECTOR_LENGTH 10

/**
* Get array length (without ending '\0')
* @param this Vector itself
* @param content Input array
* @return Length of char array
*/
unsigned long long getArrayLength(struct Vector *this, const void *content) {
	unsigned long long idx = 1;
	for (; ((char*)content)[idx * this->typeLength] != '\0'; idx++);
	return idx;
}

/**
* Expand memory when required
* @param this Vector itself
* @param newLength How long that new array is (by element, not byte)
* @return Status, returning MemoryError when realloc failed
*/
Signal expandMemory(Vector* this, unsigned long long newLength) {
	// Calculate new buffer size
	unsigned long long currBufferSize = this->buffer_size;
	this->buffer_size = newLength * this->typeLength * 2;   // expanding rate: 2
	char *newBuffer = realloc(this->array, this->typeLength * this->buffer_size);

	// "realloc" may produce null pointer, caused by insufficient memory.
	if (newBuffer == NULL) {
		this->buffer_size = currBufferSize;
		return MemoryError;
	}

	// Init new buffer area
	memset(newBuffer + (this->typeLength * this->length), 0, this->typeLength * (this->buffer_size - this->length));
	this->array = newBuffer;
	return Success;
}

/**
* Append an array of stuff, according to Vector "typeLength"
* @param this Vector itself
* @param array New array for appending
* @return Status
*/
Signal append_multi_vector(Vector* this, const void *array) {
	unsigned long long extraLength = this->getArrayLength(this, array);
	unsigned long long newLength = this->length + extraLength;
	if (newLength * this->typeLength + 1 > this->buffer_size)
		this->expandMemory(this, newLength);

	memcpy(((char *)this->array) + (this->typeLength * this->length), array, this->typeLength * extraLength);
	this->length = newLength;
	return Success;
}

/**
* Append a single unit of stuff, according to Vector "typeLength"
* @param this Vector itself
* @param item New item for appending, please provide address of the data/addr!
* @return Status
*/
Signal append_vector(Vector* this, const void *item) {
	void *merged = malloc(this->typeLength + 1);
	memset(merged, 0, this->typeLength + 1);
	memcpy(merged, item, this->typeLength);

	return this->appendMulti(this, merged);
}

/**
* Clear Vector
* @param this Vector itself
* @return Status
*/
Signal clear_vector(Vector *this) {
	// Free Memory
	if (this->deepFree)  // Only if storing pointers do this clear!!
	{
		for (size_t i = 0; i < this->length; i++)
			free(*((void**)this->get(this, i)));
	}
	free(this->array);

	// Reset Params
	this->length = 0;
	this->buffer_size = MINIMUM_VECTOR_LENGTH;

	// Relocate
	this->array = malloc(this->typeLength * this->buffer_size);
	memset(this->array, 0, this->typeLength * this->buffer_size);
	return Success;
}

/**
* Get Vector element
* @attention This "array" pointer is pointing to the first address of inner array,
* Which means you have to consider if there is any necessity for casting type.
*  - For saving actual value in inner array, casting * once for real value, direct using for string type "char*";
*  - Instance: vector->get(vector, i) => "Hello World!"; *((char*)vector->get(vector, i)) => 'H'
*  - For saving address of serial structs, casting * once for real address.
*  - Instance: *((STRUCT_TYPE**)vector->get(vector, i)) => 0x112...(real address of the struct)
* @param this Vector itself
* @param idx Index of target item, counting by multiplying "typeLength"
* @return First address of item
*/
void *get_vector(Vector* this, unsigned long long idx) {
	return ((char *)this->array) + (this->typeLength * idx);
}

void *begin_vector(Vector* this) {
	return this->array;
}

void *end_vector(Vector* this) {
	return this->get(this, this->length);
}

void delete_vector(Vector* this) {
	// Free Memory
	if (this->deepFree)  // Only if storing pointers do this clear!!
	{
		for (size_t i = 0; i < this->length; i++)
			free(*((void**)this->get(this, i)));
	}
	free(this->array);
	free(this);
}

Vector *Vector_GetInstance(int typeLength) {
	Vector *instance = malloc(sizeof(Vector));
	instance->typeLength = typeLength;
	instance->buffer_size = MINIMUM_VECTOR_LENGTH;
	instance->length = 0;
	instance->array = malloc(instance->typeLength * instance->buffer_size);
	instance->deepFree = false;

	instance->getArrayLength = &getArrayLength;
	instance->expandMemory = &expandMemory;
	instance->append = &append_vector;
	instance->appendMulti = &append_multi_vector;
	instance->clear = &clear_vector;
	instance->get = &get_vector;
	instance->begin = &begin_vector;
	instance->end = &end_vector;
	instance->delete = &delete_vector;

	return instance;
}