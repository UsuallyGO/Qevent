
#ifndef _QEVENT_CORE_H_
#define _QEVENT_CORE_H_

#define QEVENT_READ    0x1
#define QEVENT_WRITE   0x2
#define QEVENT_IO      (QEVENT_READ|QEVENT_WRITE)
#define QEVENT_TIME    0x4
#define QEVENT_SIGNAL  0x8
#define QEVENT_PERSIST 0x10

#define QEVENT_BUS_RUN  0x1
#define QEVENT_BUS_STOP 0x0

#define QEVENT_SUCCESS        0
#define QEVENT_ERR_INVAL     -1
#define QEVENT_ERR_NOTFOUND  -2
#define QEVENT_ERR_NOMEM     -3
#define QEVENT_ERR_DUPLICATE -4
#define QEVENT_ERR_BADOBJ    -5
#define QEVENT_ERR_SYSTEM    -6

typedef struct qevent_bus_config_t qevent_bus_config;
typedef struct qevent_bus_t qevent_bus;
typedef struct qevent_t qevent;
typedef struct qmin_heap_t qmin_heap;
typedef struct timeval qtime;
typedef struct busopt_t busopt;

typedef void (*callback)(int fd, int type, void *arg);

typedef struct qevent_list_t qevent_list;

typedef struct qbus_sig_t qbus_sig;
typedef struct signal_opt_t sigopt;

#endif //!_QEVENT_CORE_H_
