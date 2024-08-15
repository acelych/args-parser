#ifndef __DS_COMMON_H
#define __DS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	false = 0,
	true = 1
} Bool;

typedef enum {
	Success = 0,
	Error = 1,
	TypeError = 2,
	FormatError = 3,
	MemoryError = 4,
	Invalid = 5,
} Signal;

#endif