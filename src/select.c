
#include <string.h>
#include <sys/select.h>

#include "qevent/qevent.h"
#include "memory-internal.h"
#include "config-internal.h"

static void select_init(qevent_bus *bus);
static void select_add(qevent_bus *bus, int fd, int type);
static void select_del(qevent_bus *bus, int fd, int type);
static void select_run(qevent_bus *bus, qtime *time);
static void select_clear(qevent_bus *bus);

#ifdef QEVENT_HAVE_SELECT
const busopt _selectopt = 
{
    select_init,
    select_add,
    select_del,
    select_run,
    select_clear,
};
#endif

struct select_data_t
{
    int m_readmax;
    int m_writemax;

    fd_set m_readset;
    fd_set m_writeset;

    fd_set m_readset_run;
    fd_set m_writeset_run;
};

typedef struct select_data_t select_data;

static void select_init(qevent_bus *bus)
{
    select_data *seldata;

    seldata = (select_data*)qmalloc(sizeof(select_data));
    if(seldata != NULL)
    {
        seldata->m_readmax = 0;
        seldata->m_writemax = 0;
        FD_ZERO(&seldata->m_readset);
        FD_ZERO(&seldata->m_writeset);
    }

    bus->m_optdata = seldata;
}

//just set the fd to fd_set
/*
static void select_add(qevent_bus *bus, qevent *event)
{
    select_data *seldata;

    seldata = (select_data*)bus->m_optdata;
    if(event->m_type & QEVENT_READ)
    {
        if(seldata->m_readmax < event->m_fd)
            seldata->m_readmax = event->m_fd;
        FD_SET(event->m_fd, &seldata->m_readset);
    }
    if(event->m_type & QEVENT_WRITE)
    {
        if(seldata->m_writemax < event->m_fd)
            seldata->m_writemax = event->m_fd;
        FD_SET(event->m_fd, &seldata->m_writeset);
    }
}
*/
static void select_add(qevent_bus *bus, int fd, int type)
{
    select_data *seldata;

    seldata = (select_data*)bus->m_optdata;
    if(type & QEVENT_READ)
    {
        if(seldata->m_readmax < fd)
            seldata->m_readmax = fd;
        FD_SET(fd, &seldata->m_readset);
    }
    if(type & QEVENT_WRITE)
    {
        if(seldata->m_writemax < fd)
            seldata->m_writemax = fd;
        FD_SET(fd, &seldata->m_writeset);
    }
}

/*
static void select_del(qevent_bus *bus, qevent *event)
{
    select_data *seldata;

    seldata = (select_data*)bus->m_optdata;
    if(event->m_type & QEVENT_READ)
    {
        if(seldata->m_readmax >= event->m_fd)
            FD_CLR(event->m_fd, &seldata->m_readset);
    }
    if(event->m_type & QEVENT_WRITE)
    {
        if(seldata->m_writemax >= event->m_fd)
            FD_CLR(event->m_fd, &seldata->m_writeset);
    }
}
*/

static void select_del(qevent_bus *bus, int fd, int type)
{
    select_data *seldata;

    seldata = (select_data*)bus->m_optdata;
    if(type & QEVENT_READ)
    {
        FD_CLR(fd, &seldata->m_readset);
    }
    if(type & QEVENT_WRITE)
    {
        FD_CLR(fd, &seldata->m_writeset);
    }
}

static void select_run(qevent_bus *bus, qtime *time)
{
    int res, maxfd, index;
    select_data *seldata;

    seldata = bus->m_optdata;
    FD_ZERO(&seldata->m_readset_run);
    FD_ZERO(&seldata->m_writeset_run);

    memcpy(&seldata->m_readset_run, &seldata->m_readset, sizeof(seldata->m_readset));
    memcpy(&seldata->m_writeset_run, &seldata->m_writeset, sizeof(seldata->m_writeset));

    maxfd = seldata->m_writemax;
    if(seldata->m_readmax > seldata->m_writemax)
        maxfd = seldata->m_readmax;

    res = select(maxfd+1, &seldata->m_readset_run, &seldata->m_writeset_run,\
                 NULL, time);

    for(index = 0; index < maxfd+1; index++)//loop, maybe serveral events triggered
    {
        res = 0;
        if(FD_ISSET(index, &seldata->m_readset_run))
            res |= QEVENT_READ;
        if(FD_ISSET(index, &seldata->m_writeset_run))
            res |= QEVENT_WRITE;

        if(res == 0)
            continue;

        if(index == bus->m_bussig->m_server)
            bus_active_sig(bus, index, res);
        else
            bus_active_io(bus, index, res);
    }

}

static void select_clear(qevent_bus *bus)
{
    select_data *sel_data;

    sel_data = (select_data*)bus->m_optdata;

    qfree(sel_data);
}
