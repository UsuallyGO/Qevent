
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>//both for linux socket
#include <signal.h>

#include "qevent/qevent.h"
#include "memory-internal.h"

void signal_add(qevent_bus *bus, qevent *event);
void signal_del(qevent_bus *bus, qevent *event);

qbus_sig _bussig;

sigopt _sigopt =
{
    signal_add,
    signal_del,
};

//write the bussig->m_server
static void signal_handler(int sig)
{
    int msg;

    msg = sig;
    send(_bussig.m_client, &msg, sizeof(msg), 0);//m_client and been connected to m_server
}

void signal_add(qevent_bus *bus, qevent *event)
{
    int sig;

    sig = event->m_fd;
    bus->m_busopt->bus_attach(bus, _bussig.m_server, QEVENT_READ);
    signal(sig, signal_handler);
}

void signal_del(qevent_bus *bus, qevent *event)
{
    signal(event->m_fd, SIG_DFL);//now, just reset to default handler, a little naive...
}

int bussig_init(qevent_bus *bus)
{
    qbus_sig *sig;
    int listener, connector, acceptor;
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    socklen_t len;

    bus->m_sigopt = &_sigopt;
    if(bus->m_sigopt == NULL)
        return QEVENT_ERR_SYSTEM;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
        return QEVENT_ERR_SYSTEM;

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0; /* kernel choose port */
    if(bind(listener, (struct sockaddr*)&listen_addr, \
             sizeof(listen_addr)) < 0)
        return QEVENT_ERR_SYSTEM;
    if(listen(listener, 1) < 0)//only one connection
        return QEVENT_ERR_SYSTEM;

    connector = socket(AF_INET, SOCK_STREAM, 0);
    if(connector < 0)
        return QEVENT_ERR_SYSTEM;

    len = sizeof(connect_addr);
    if(getsockname(listener, (struct sockaddr*)&connect_addr,\
                    &len) < 0)
        return QEVENT_ERR_SYSTEM;

    if(connect(connector, (struct sockaddr*)&connect_addr,\
                sizeof(connect_addr)) < 0)
        return QEVENT_ERR_SYSTEM;

    len = sizeof(listen_addr);
    acceptor = accept(listener, (struct sockaddr*)&listen_addr, &len);
    if(acceptor < 0)
        return QEVENT_ERR_SYSTEM;

    close(listener);//what we need is connector and  acceptor

    _bussig.m_server = acceptor;
    _bussig.m_client = connector;

    bus->m_bussig = &_bussig;
    return QEVENT_SUCCESS;
}

//mainly to close the m_server and m_client
void bussig_clear(qbus_sig *sig)
{
    close(sig->m_server);
    close(sig->m_client);
}
