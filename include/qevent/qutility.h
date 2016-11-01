
#ifndef _QEVENT_UTILITY_H_
#define _QEVENT_UTILITY_H_

#define QMINHEAP_INITIAL_SIZE  100
#define QMINHEAP_INCREASE_SIZE 100

#include "qevent/qcore.h"

struct qevent_list_t
{
    qevent *m_head;
    qevent *m_tail;
    int m_count;
};

#endif //! _QEVENT_UTILITY_H_
