
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "qevent/qevent.h"

void readFunc(int fd, int type, void *arg)
{
    qevent *event;
    int ran;
    static int count = 0;

    event = (qevent*)arg;
    ran = rand()%10 + 1;
    count++;
    printf("In readFunc, next time gap:%d\n", ran);
    if(count < 10)
    {
        qevent_set_time(event, ran, 0);
        qevent_bus_attach(event->m_bus, event);
    }
}

int main()
{
    int res, fd;
    long msg;
    qevent *event;
    qevent_bus *bus;
    qtime *time;

    bus = qevent_bus_new(NULL);
    event = qevent_new();

    qevent_set(event, -1, QEVENT_TIME, readFunc, event);
    qevent_set_time(event, 2, 0);
    qevent_bus_attach(bus, event);

    qevent_bus_run(bus);

    qevent_bus_destroy_all();

    qmemInfoDebug(0);
    return 0;
}
