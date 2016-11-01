
#include <stdio.h>
#include <signal.h>
#include "qevent/qevent.h"

static void signal_cb(int fd, int type, void *arg)
{
    qevent *event;
    static int count = 0;
	
    event = (qevent*)arg;
    printf("Signal callback, signal:%d type:%d count:%d\n", fd, type, count);

    count++;
    if(count < 5)
        qevent_bus_attach(event->m_bus, event);
    else
        printf("More than 5 times of signal, not any more.\n");
}

int main()
{
    qevent *event;
    qevent_bus *bus;

    event = qevent_new();
    bus = qevent_bus_new(NULL);

    qevent_set(event, SIGINT, QEVENT_SIGNAL, signal_cb, event);
    qevent_bus_attach(bus, event);
    printf("Signal has been attached to the event bus.\n");
    printf("Waiting for signal triger...\n");

    qevent_bus_run(bus);

    qevent_bus_destroy_all();

    qmemInfoDebug(0);
    return 0;
}
