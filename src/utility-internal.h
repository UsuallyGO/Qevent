
#ifndef _QEVENT_UTILITY_INTERNAL_H_
#define _QEVENT_UTILITY_INTERNAL_H_

void qevent_list_init(qevent_list *list);

qevent* qevent_list_remove_byfd(qevent_list *list, int fd);

void qevent_list_add_tail(qevent_list *list, qevent *event);

qevent* qevent_list_remove_head(qevent_list *list);

int qmin_heap_init(qmin_heap *heap);

qevent* qmin_heap_top(qmin_heap *heap);

int qmin_heap_insert(qmin_heap *heap, qevent *event);

int qmin_heap_remove(qmin_heap *heap, qevent *event);

void qtime_set(qtime *time, int sec, int usec);

void qtime_sub(qtime *sub1, qtime *sub2, qtime *diff);

void qtime_add(qtime *add1, qtime *add2, qtime *sum);

int qtime_sign(qtime *time);

#endif //!_QEVENT_UTILITY_INTERNAL_H_
