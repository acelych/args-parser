#ifndef __DS_VECTOR_H
#define __DS_VECTOR_H

#include "ds_common.h"

/*------------------------------ Vector ------------------------------*/

/**
* Vector Struct for C language
* @attention This vector is implemented based on a linear table.
* The whole data structure is be like: [[DATA1], [DATA2], ... , \0]
* @param array Pointing beginning of inner array
* @param typeLength Length of unit of type of Vector
* @param length Length of array
* @param buffer_size Actual buffer size created by malloc func
* @param expandingMemory Function: Requiring more memory when needed
* @param append Function: Append single unit
*/
typedef struct Vector {
	void *array;
	int typeLength;
	unsigned long long length, buffer_size;
	Bool deepFree;

	unsigned long long(*getArrayLength)(struct Vector*, const void*);
	Signal(*expandMemory)(struct Vector*, unsigned long long);
	Signal(*append)(struct Vector*, const void*);
	Signal(*appendMulti)(struct Vector*, const void*);
	Signal(*clear)(struct Vector*);
	void  *(*get)(struct Vector*, unsigned long long idx);
	void  *(*begin)(struct Vector*);
	void  *(*end)(struct Vector*);
	void (*delete)(struct Vector*);
} Vector;

Vector *Vector_GetInstance(int typeLength);

#endif