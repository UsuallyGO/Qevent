
#ifndef _QEVENT_SIGNAL_H_
#define _QEVENT_SIGNAL_H_

struct qbus_sig_t
{
    int m_server;
    int m_client;
};

struct signal_opt_t
{
    void (*sig_add)(qevent_bus *, qevent *);
    void (*sig_del)(qevent_bus *, qevent *);
};

#endif //!_QEVENT_SIGNAL_H_

