
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

typedef void* memory_block;
typedef struct memory_suite_t memory_suite;

struct memory_suite_t
{
    size_t        m_size;
    memory_block  m_block;
    unsigned int  m_id;
    int           m_line;
    const char*   m_file;
    memory_suite* m_next;
};

static memory_suite* memList = NULL;
static unsigned int memCount = 0; //never decrease

static void _set_memory_suite(memory_suite *suite, memory_block block,
                         size_t size, int line, const char* file)
{
    suite->m_size = size;
    suite->m_block = block;
    suite->m_line = line;
    suite->m_file = file;

    //Lock on here???
    suite->m_id = memCount++;
    suite->m_next = memList;
    memList = suite;
    //UnLock here???    
}

static memory_suite* _remove_memory_suite(memory_block block)
{
    memory_suite *prev, *cur;

    prev = NULL;
    for(cur = memList; cur != NULL; cur = cur->m_next)
    {
        if(cur->m_block == block)
            break;
        prev = cur;
    }

    if(cur != NULL)
    {
        if(cur == memList)
            memList = memList->m_next;
        else
            prev->m_next = cur->m_next;
    }

    return cur;
}

static memory_suite* _get_memory_suite(void *ptr)
{
    memory_suite *suite = NULL;

    for(suite = memList; suite != NULL; suite = suite->m_next)
        if(suite->m_block == ptr)
            break;

    return suite;
}

void* qevent_malloc(size_t size, int line, const char *file)
{
    memory_block block = NULL;
    memory_suite *suite;

    suite = (memory_suite*)malloc(sizeof(memory_suite));
    if(suite == NULL)
        return NULL;

    block = (memory_block)malloc(size);
    if(block == NULL)
    {
        free(suite);
        return NULL;
    }

    _set_memory_suite(suite, block, size, line, file);
    return block;
}

void* qevent_realloc(void *ptr, size_t size, int line, char *file)
{
    memory_suite *suite;

    suite = _get_memory_suite(ptr);
    if(suite == NULL)
        return NULL;//this block is not in
    else
    {
        suite->m_block = (memory_block)realloc(suite->m_block, size);
        if(suite->m_block == NULL)
            suite->m_size = 0;
        else
            suite->m_size = size;

        suite->m_line = line; 
        suite->m_file = file;
    }

    return suite->m_block;
}

void qevent_free(void *ptr)
{
    memory_suite *suite;

    if(ptr != NULL)
    {
        suite = _remove_memory_suite(ptr);
        if(suite != NULL)
        {
            free(ptr);
            free(suite);
        }
    }
}

void qmemInfoDebug(unsigned int since)
{
    memory_suite *suite;
    unsigned int count = 0;

    printf("\n");
    //Lock on here???
    for(suite = memList; suite != NULL; suite = suite->m_next)
    {
        if(suite->m_id >= since)
        {
            printf("Memory leak, created at:%s, %d line.\n",
                    suite->m_file, suite->m_line);
            count++;
        }
    }

    printf("Memory leak check, %d blocks detected.\n", count);
}