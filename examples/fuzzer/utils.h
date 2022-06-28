#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WORD_BIT 4

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
/*
 * swap - swap value of @a and @b
 */
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-MAX_ERRNO)
static inline long IS_ERR(unsigned long x)
{
	return IS_ERR_VALUE(x);
}

#define __stringify_1(x...)     #x
#define __stringify(x...)       __stringify_1(x)

#define unreachable() do { } while (1)

#define RAND_ELEMENT(_array, _element) \
	_array[rnd() % ARRAY_SIZE(_array)]._element

#define RAND_ARRAY(_array) _array[rnd() % ARRAY_SIZE(_array)]

#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)