
#ifndef _QEVENT_EVENT_H_
#define _QEVENT_EVENT_H_
/* A event bus just like a bus in computer, ether data bus or address bus. Each event
   just like a device attached to the bus.
 */
 
#include <sys/time.h>
#include <signal.h>

#include "qcore.h"
#include "qsignal.h"
#include "qutility.h"

struct qmin_heap_t
{
    qevent **m_heap;
    int m_index; //awayls points to the next available entry
    int m_numbers; //total entries(empty and non-empty)
};

struct busopt_t
{
    void (*bus_init)(qevent_bus *);
    void (*bus_attach)(qevent_bus*, int, int);
    void (*bus_detach)(qevent_bus*, int, int);
    void (*bus_run)(qevent_bus*, qtime *);
    void (*bus_clear)(qevent_bus*);
};

struct qevent_bus_config_t
{
    //user can set the priority
    int m_select;
    int m_poll;
    int m_kpoll;
    int m_epoll;
    int m_kqueue;
};

struct qevent_bus_t
{
    char *m_optname;

    const busopt *m_busopt; //must be const here
    void *m_optdata;//each option has its specify data

    const sigopt *m_sigopt;
    qbus_sig *m_bussig;

    qevent_list m_siglist;
    qevent_list m_iolist;
    qevent_list m_activelist;

    qtime m_time;
    qmin_heap m_minheap;

    int m_running;
    int m_errno;
    char *m_errmsg;
    qevent_bus *m_next;//all buses in a list
};

struct qevent_t
{
    int m_fd;
    int m_type;
    int m_status;
    void *m_arg;

    qevent_bus *m_bus;//used when event reattached in callback
    qtime m_time;
    callback m_callback;

    qevent *m_next;//arranged in list
};

qevent_bus_config* qevent_bus_config_new();

qevent_bus* qevent_bus_new(qevent_bus_config *config);

qevent* qevent_new();

int qevent_set(qevent *event, int fd, int type, callback funcp, void *arg);

void qevent_set_time(qevent *event, int sec, int usec);

int qevent_bus_attach(qevent_bus *bus, qevent *event);

void bus_active_io(qevent_bus *bus, int fd, int status);

void bus_timeout_process(qevent_bus *bus);

void qevent_bus_run(qevent_bus *bus);

void qmemInfoDebug(unsigned int since);

#endif //!_QEVENT_EVENT_H_
