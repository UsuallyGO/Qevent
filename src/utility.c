/*                                                       _ _
 * Wahaha...What do you mean, body? Are you kidding me?? @_@
 * Who wrote these rubbish code... garbo! A waste of time, of water, of bread...
 */

#include <string.h>

#include "qevent/qevent.h"
#include "memory-internal.h"

void qevent_list_init(qevent_list *list)
{
    list->m_head = NULL;
    list->m_tail = NULL;
    list->m_count = 0;
}

//clear all the events
void qevent_list_clear(qevent_list *list)
{
    qevent *cur;

    while((cur = list->m_head) != NULL)
    {
        list->m_head = cur->m_next;
        qfree(cur);
    }

    list->m_head = list->m_tail = NULL;
    list->m_count = 0;
}

qevent* qevent_list_remove_byfd(qevent_list *list, int fd)
{
    qevent *cur, *prev;

    if(list->m_count == 0)
        return NULL;

    cur = NULL;
    if(list->m_head->m_fd == fd)
    {
        cur = list->m_head;
        list->m_head = cur->m_next;
        if(list->m_count == 1)
            list->m_tail = NULL;
        list->m_count--;
        return cur;
    }

    prev = list->m_head;
    cur = prev->m_next;
    while(cur != NULL)
    {
        if(cur->m_fd == fd)
        {
            prev->m_next = cur->m_next, cur->m_next = NULL;
            if(cur == list->m_tail)
                list->m_tail = prev;
             list->m_count--;
             break;
        }
        else
            prev = cur, cur = cur->m_next;
    }

    return cur;
}

void qevent_list_add_tail(qevent_list *list, qevent *event)
{
    event->m_next = NULL;

    if(list->m_count == 0)
        list->m_head = event;
    else
        list->m_tail->m_next = event;
    list->m_tail = event;
    list->m_count++;
}

qevent* qevent_list_remove_head(qevent_list *list)
{
    qevent *tmp;

    if(list->m_count == 0)
        return NULL;

    tmp = list->m_head;
    list->m_head = tmp->m_next;
    if(list->m_count == 1)
        list->m_tail = NULL;    

    list->m_count--;
    return tmp;
}

int qevent_list_find(qevent_list *list, qevent *event)
{
    qevent *cur;

    for(cur = list->m_head; cur != NULL; cur = cur->m_next)
        if(cur == event)
            return QEVENT_SUCCESS;
     
    return QEVENT_ERR_NOTFOUND;
}

////////////////////////////////////////////////////////////////////////////////
//
int qmin_heap_init(qmin_heap *heap)
{
    heap->m_heap = (qevent**)qmalloc(sizeof(qevent*)*QMINHEAP_INITIAL_SIZE);
    if(heap->m_heap == NULL)
        return QEVENT_ERR_NOMEM;

    heap->m_index = 0;
    heap->m_numbers = QMINHEAP_INITIAL_SIZE;
    memset(heap->m_heap, 0, sizeof(qevent*)*QMINHEAP_INITIAL_SIZE);
    return QEVENT_SUCCESS;
}

//set the heap to the initial state
void qmin_heap_clear(qmin_heap *heap)
{
    qfree(heap->m_heap); //do not free the event
    heap->m_heap = NULL;
    heap->m_index = 0;
    heap->m_numbers = 0;
}

qevent* qmin_heap_top(qmin_heap *heap)
{
    return (qevent*)heap->m_heap[0];
}

int qmin_heap_insert(qmin_heap *heap, qevent *event)
{
    int index, parent;
    qevent *tmp;

    if(heap->m_index >= heap->m_numbers)//heap if full, try to resize
    {
        //index reused here to calculate the new size
        index = (heap->m_numbers + QMINHEAP_INCREASE_SIZE)*sizeof(qevent*);
        heap->m_heap = (qevent**)qrealloc(heap->m_heap, index);
        //check heap->m_heap here...
        heap->m_heap += QMINHEAP_INCREASE_SIZE;
    }

    index = heap->m_index;//heap->m_index is the first empty entry
    parent = (index - 1)/2;
    heap->m_heap[index] = event;
    while(parent > 0)
    {
        if(qtime_cmp(heap->m_heap[parent]->m_time,
               heap->m_heap[index]->m_time) > 0)
        {
            tmp = heap->m_heap[parent];
            heap->m_heap[parent] = heap->m_heap[index];
            heap->m_heap[index] = tmp;

            index = parent;
            parent = (parent - 1)/2;
        }
        else//parent is earlier than  index
            break;
    }

    heap->m_index++;

    return QEVENT_SUCCESS;
}

int qmin_heap_remove(qmin_heap *heap, qevent *event)
{
    int index, child, swap;
    qevent *tmp;

    //find event in heap first
    for(index = 0; index < heap->m_index; index++)
        if(heap->m_heap[index] == event)
            break;

    if(index == heap->m_index)
        return QEVENT_ERR_NOTFOUND;//event not in the heap

    //swap event and the last event
    tmp = heap->m_heap[index];
    heap->m_heap[index] = heap->m_heap[heap->m_index-1];
    heap->m_heap[heap->m_index-1] = tmp;

    heap->m_heap[heap->m_index-1] = NULL;//remove from the heap
    heap->m_index--;//need to resize here???

    //fix from the root
    for(index = 0; index < heap->m_index; /**/)
    {
        swap = 0;
        child = index*2 + 1;//points to left child
        if(child >= heap->m_index)
            break;
        if(qtime_cmp(heap->m_heap[index]->m_time,
                 heap->m_heap[child]->m_time) > 0)
            swap = 1;
        else if(qtime_cmp(heap->m_heap[index]->m_time, 
                     heap->m_heap[child+1]->m_time) > 0)
            swap = 1, child++;

        if(swap)//need to swap son and parent
        {
            tmp = heap->m_heap[index];
            heap->m_heap[index] = heap->m_heap[child];
            heap->m_heap[child] = tmp;

            index = child;
        }
        else
            break;//don't need to fix
    }

    return QEVENT_SUCCESS;
}

/* if time1 is eariler than time2, return < 0
 * if time1 is equal to time2, return 0
 * if time1 is later than time2, return > 0
 */
int qtime_cmp(qtime *time1, qtime *time2)
{
    if(time1->tv_sec < time2->tv_sec)
        return -1;
    else if(time1->tv_sec > time2->tv_sec)
        return 1;

    if(time1->tv_usec < time2->tv_usec)
        return -1;
    else if(time1->tv_usec > time2->tv_usec)
        return 1;
    else
        return 0;	
}

void qtime_set(qtime *time, int sec, int usec)
{
    time->tv_sec = sec;
    time->tv_usec = usec;
}

void qtime_clear(qtime *time)
{
    timerclear(time);//just for linux. macro
}

//time.tv_sec and time.tv_usec are long int type
void qtime_sub(qtime *sub1, qtime *sub2, qtime *diff)
{
    diff->tv_sec = sub1->tv_sec - sub2->tv_sec;
    diff->tv_usec = sub1->tv_usec - sub2->tv_usec;
    if(diff->tv_sec > 0)
    {
        if(diff->tv_usec < 0)
        {
            diff->tv_sec--;
            diff->tv_usec += 1000000;
        }
    }
    else if(diff->tv_sec < 0)
    {
        if(diff->tv_usec > 0)
        {
            diff->tv_sec++;
            diff->tv_usec = 1000000 - diff->tv_usec;
        }
    }
}

void qtime_add(qtime *add1, qtime *add2, qtime *sum)
{
    sum->tv_sec = add1->tv_sec + add2->tv_sec;
    sum->tv_usec = add1->tv_usec + add2->tv_usec;
}

int qtime_sign(qtime *time)
{
    if(time->tv_sec < 0)
        return -1;
    if(time->tv_sec == 0 && time->tv_usec < 0)
        return -1;
    if(time->tv_sec == 0 && time->tv_usec == 0)
        return 0;

    return 1;
}
