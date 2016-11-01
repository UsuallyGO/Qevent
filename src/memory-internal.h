
#ifndef _QEVENT_MEMORY_INTERNAL_H_
#define _QEVENT_MEMORY_INTERNAL_H_

#include <stdlib.h>

#define qmalloc(x)        qevent_malloc(x, __LINE__, __FILE__)
#define qfree(x)          qevent_free(x)
#define qrealloc(x,s)     qevent_realloc(x, s, __LINE__, __FILE__) 

void* qevent_malloc(size_t size, int line, const char *file);

void qevent_free(void *ptr);

void* qevent_realloc(void *ptr, size_t size, int line, char *file);

#endif //!_QEVENT_MEMORY_INTERNAL_H_
