/***********************************************************
 * What a fucking world!  To write your program robust, ****
 * please drink more Robust, more Robust...             ****
 ***********************************************************
 ****             Sing a song for me, please...        *****
 ***********************************************************
 *****              The clouds of hometown             *****
 *****                                                 *****
 *****         With clouds of hometown drifting        *****
 *****             they keep calling on me             *****
 *****                Come back please                 *****
 *****          Stop roaming all over the world        *****
 *****              With the heavy steps               *****
 *****           the way home seems endless            *****
 *****             When the breeze passes              *****
 *****        I smell the fragrance of my soil home    *****
 *****                Come back please                 *****
 *****                 My dear ranger                  *****
 *****                Come back please                 *****
 *****           So tired of a wandering life          *****
 *****               I am so exhausted                 *****
 *****             eyes filled with sad tears          *****
 *****       Only can the wind and cloud of hometown   *****
 *****               heal my broken heart              *****
 *****          Filled with lofty feelings before      *****
 *****            I came back with an empty bag        *****
 *****       Only can the wind and cloud of hometown   *****
 *****               heal my broken heart              *****
 ***********************************************************
 */

#include <sys/time.h>

#include "qevent/qevent.h"
#include "memory-internal.h"
#include "utility-internal.h"
#include "config-internal.h"

#define ERRMSG_NO_ERROR     0
#define ERRMSG_NO_OPTION    1
#define ERRMSG_NO_NAME      2
#define ERRMSG_BAD_SIG      3
#define ERRMSG_BAD_TIMEHEAP 4
#define ERRMSG_BAD_EVENT    5

extern busopt _selectopt;

const static busopt* _busopt[] = 
{
#ifdef QEVENT_HAVE_SELECT
    &_selectopt,
#endif
    //pollopt,
    //epollopt,
    //kpollopt,
    //kqueueopt,
    NULL,
};

static char _optname[][10] = 
{
    "select",
    "poll",
    "epoll",
    "kpoll",
    "kqeueu",
};

static char _errmsg[][50] = 
{
    "bus has no error",
    "bus has no event option, check your os",
    "bus option has no name",
    "bus can not work well on signal",
    "bus has no time heap",
    "bus has unknown event",
};

static qevent_bus *_buslist = NULL;
static int _buscounter = 0;


qevent_bus_config* qevent_bus_config_new()
{
    int index;
    qevent_bus_config *config;

    config = (qevent_bus_config*)qmalloc(sizeof(qevent_bus_config));
    //check here...
    return config;
}

static void _bus_errmsg_set(qevent_bus *bus, int errno)
{
    if(bus->m_errno == ERRMSG_NO_ERROR)//don't overlay the error msg
    {
        bus->m_errno = errno;
        bus->m_errmsg = _errmsg[errno];
    }
}

static qevent_bus* _bus_new_with_config(qevent_bus_config *config)
{
    qevent_bus_config *configure;
    qevent_bus *bus;
    int index, res;
    int* conf;

    configure = config;
    if(configure == NULL)
        ;//configure = qevent_bus_config_new();

    //set configure here...
    //destroy configure here...

    bus = (qevent_bus*)qmalloc(sizeof(qevent_bus));
    if(bus == NULL)
        goto Err_fatal;
   
    //set bus with config...
    //free configure here

    for(index = 0; _busopt[index] != NULL; index++)
    {
        bus->m_busopt = _busopt[index];
        bus->m_optname = _optname[index];
    }

    if(bus->m_busopt == NULL)//no event option, cannot work, fatal error
        goto Err_fatal;
    if(bus->m_optname == NULL)//just no name, not fatal
        _bus_errmsg_set(bus, ERRMSG_NO_NAME);

    bus->m_running = QEVENT_BUS_RUN;
    bus->m_optdata = NULL;
    bus->m_busopt->bus_init(bus);//should set bus->m_optdata inside
    if(bus->m_optdata == NULL)//option data is necessary, even it is emptry, must be set
        goto Err_fatal;

    res = bussig_init(bus);
    if(res != QEVENT_SUCCESS)//only signal cannot work well, no fatal
        _bus_errmsg_set(bus, ERRMSG_BAD_SIG);

    qevent_list_init(&bus->m_siglist);
    qevent_list_init(&bus->m_iolist);
    qevent_list_init(&bus->m_activelist);

    res = qmin_heap_init(&bus->m_minheap);
    if(res != QEVENT_SUCCESS)//no timer minheap, timeout event cannot work well
        _bus_errmsg_set(bus, ERRMSG_BAD_TIMEHEAP);

    bus->m_next = _buslist;
    _buslist = bus;
    _buscounter++;

    return bus;

Err_fatal:
    if(bus != NULL)//notice that: bus->m_optdata must be NULL here
        qfree(bus);
    return NULL;
}

qevent_bus* qevent_bus_new(qevent_bus_config *config)
{
    qevent_bus *bus;

    bus = _bus_new_with_config(config);//bus has been checked inside
    return bus;
}

qevent* qevent_new()
{
    qevent *event;

    event = (qevent*)qmalloc(sizeof(qevent));
    if(event == NULL)
        return NULL;

    event->m_fd = -1;
    event->m_type = -1;
    event->m_status = -1;
    event->m_arg = NULL;
    qtime_clear(&event->m_time);
    event->m_callback = NULL;
    event->m_next = NULL;
    event->m_bus = NULL;

    return event;
}

int qevent_set(qevent *event, int fd, int type, callback funcp, void *arg)
{
    if(event == NULL)
        return QEVENT_ERR_INVAL;

    event->m_fd = fd;
    event->m_type = type;
    event->m_arg = arg;

    qtime_clear(&event->m_time);    
    event->m_callback = funcp;
    event->m_next = NULL;

    return QEVENT_SUCCESS;
}

void qevent_set_time(qevent *event, int sec, int usec)
{
    qtime_set(&event->m_time, sec, usec);
}

void _bus_signal_attach(qevent_bus *bus, qevent *event)
{
    qevent_list_add_tail(&bus->m_siglist, event);
    bus->m_sigopt->sig_add(bus, event);    
}

void _bus_io_time_attach(qevent_bus *bus, qevent *event)
{
    if(event->m_type & QEVENT_IO)
        bus->m_busopt->bus_attach(bus, event->m_fd, event->m_type);
    if(event->m_type & QEVENT_TIME)
        qmin_heap_insert(&bus->m_minheap, event);

    qevent_list_add_tail(&bus->m_iolist, event);
}

int qevent_bus_attach(qevent_bus *bus, qevent *event)
{
    int res;
    qtime now;

    if(event->m_bus != NULL && event->m_bus != bus)//event can be only attached to one bus
        return QEVENT_ERR_BADOBJ;
    //check event wether already attached to the bus or not
    res = qevent_list_find(&bus->m_iolist, event);
    if(res == QEVENT_SUCCESS)
        return QEVENT_ERR_DUPLICATE;
    res = qevent_list_find(&bus->m_siglist, event);
    if(res == QEVENT_SUCCESS)
        return QEVENT_ERR_DUPLICATE;
    //bus->m_activelist must be NULL here...

    res = QEVENT_SUCCESS;
    event->m_bus = bus;

    //Need to rewrite here... can't deal with timeout IO or timeout signal
    if(event->m_type & QEVENT_IO
       || event->m_type & QEVENT_TIME)//read or write
    {
        if(event->m_type & QEVENT_TIME)
        {
            gettimeofday(&now, NULL);
            qtime_add(&now, &event->m_time, &event->m_time);//change to absolute time
        }
        _bus_io_time_attach(bus, event);
    }
    else if(event->m_type & QEVENT_SIGNAL)
    {
        _bus_signal_attach(bus, event);
    }
    else
    {
        res = QEVENT_ERR_BADOBJ;//serious error here...
        event->m_bus = NULL;
    }

    return res;
}

void bus_active_sig(qevent_bus *bus, int fd, int status)
{
    qevent *event;
    int signal, res;

    res = recv(fd, &signal, sizeof(signal), 0);
    if(res <= 0)
        ;//serious error here
 
    event = qevent_list_remove_byfd(&bus->m_siglist, signal);//in siglist, signal is the list
    if(event == NULL)
        _bus_errmsg_set(bus, ERRMSG_BAD_EVENT);
    else
    {
        event->m_status = status;
        qevent_list_add_tail(&bus->m_activelist, event);
        bus->m_busopt->bus_detach(bus, fd, QEVENT_READ);
    }
}

void bus_active_io(qevent_bus *bus, int fd, int status)
{
    qevent *event;

    event = qevent_list_remove_byfd(&bus->m_iolist, fd);
    if(event == NULL)
        _bus_errmsg_set(bus, ERRMSG_BAD_EVENT);
    else
    {
        event->m_status = status;
        qmin_heap_remove(&bus->m_minheap, event);//should no error here
        qevent_list_add_tail(&bus->m_activelist, event);
    }
}

void bus_timeout_process(qevent_bus *bus)
{
    qtime now;
    qevent *event;

    gettimeofday(&now, NULL);
    while((event = qmin_heap_top(&bus->m_minheap)) != NULL)
    {
        if(qtime_cmp(&now, &event->m_time) < 0)//no event timeout
            break;
        //move the event to active list
        qmin_heap_remove(&bus->m_minheap, event);
        qevent_list_remove_byfd(&bus->m_iolist, event->m_fd);
        qevent_list_add_tail(&bus->m_activelist, event);
    }
}

void qevent_bus_run(qevent_bus *bus)
{
    qevent *event;
    qtime *time;
    qtime now, tmp;

    while(bus->m_running == QEVENT_BUS_RUN)
    {
        time = NULL;
        event = qmin_heap_top(&bus->m_minheap);
        if(event != NULL)
        {
            gettimeofday(&now, NULL);
            qtime_sub(&event->m_time, &now, &tmp);
            if(qtime_sign(&tmp) < 0)
            {
                qtime_clear(&tmp);
                tmp.tv_usec = 10000; //10ms
            }
            time = &tmp;
        }

        bus->m_busopt->bus_run(bus, time);
        bus_timeout_process(bus);

        while(event = qevent_list_remove_head(&bus->m_activelist))
        {
            if(event->m_type & QEVENT_IO)//pure timeout has no io event
                bus->m_busopt->bus_detach(bus, event->m_fd, event->m_type);//before callback, event may reinsert in callback
            else if(event->m_type & QEVENT_SIGNAL)
                bus->m_sigopt->sig_del(bus, event);
            (event->m_callback)(event->m_fd, event->m_type, event->m_arg);
        }
    }
}

void qevent_bus_stop(qevent_bus *bus)
{
    if(bus == NULL)
         return;

    bus->m_running = QEVENT_BUS_STOP;
}

void _bus_destroy(qevent_bus *bus)
{
    bus->m_busopt->bus_clear(bus); //main to clear the option data
    qmin_heap_clear(&bus->m_minheap);
    qevent_list_clear(&bus->m_siglist);
    qevent_list_clear(&bus->m_iolist);
    qevent_list_clear(&bus->m_activelist);
    bussig_clear(bus->m_bussig);

    qfree(bus);
}

void qevent_bus_destroy(qevent_bus *bus)
{
    qevent_bus *cur, *prev;

    for(cur = _buslist; cur != NULL; cur = cur->m_next)
    {
        if(cur == bus)
            break;
        prev = cur;
    }

    if(cur != NULL)
    {
        if(cur == _buslist)
		    _buslist = _buslist->m_next;
        else
            prev->m_next = cur->m_next;
    }

    //bus has removed from the bus list
    _bus_destroy(bus);
}

void qevent_bus_destroy_all()
{
    qevent_bus *bus;

    while(_buslist != NULL)
    {
        bus = _buslist;
        _buslist = _buslist->m_next;
        _bus_destroy(bus);
    }
}
