#ifndef __DS_STRING_H
#define __DS_STRING_H

#include "ds_vector.h"
#include <stdarg.h>

/*------------------------------ String ------------------------------*/

typedef struct String {
	Vector *parent;

	Signal(*append)(struct String*, const char*, ...);
	Signal(*appendChar)(struct String*, const char);
	Signal(*clear)(struct String*);

	unsigned long long(*length)(struct String*);
	char* (*begin)(struct String*);
	char* (*end)(struct String*);
	char* (*chrCopy)(struct String*);
	void  (*delete)(struct String*);
} String;

String *String_GetInstance(const char *content);

#endif