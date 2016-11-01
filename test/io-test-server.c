
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "qevent/qevent.h"

#define IP_ADDRESS "127.0.0.1"
#define PORT 8787
#define MAX_LINE 1024
#define LISTEN 5
#define SIZE 10

void recv_msg(int fd, int type, void *arg)
{
    qevent *event;
    int client_fd, n;
    char buf[MAX_LINE];

    event = (qevent*)arg;
    client_fd = event->m_fd;

    sprintf(buf, "Empty buffer.\n");
    n = recv(client_fd, buf, MAX_LINE, 0);
    printf("recv msg:%s\n", buf);

    qevent_bus_attach(event->m_bus, event);
}

void listenFunc(int fd, int type, void* arg)
{
    qevent *event, *client_event;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    int client_fd = -1;
    int server_fd = -1;
    int index;

    event = (qevent*)arg;
    server_fd = event->m_fd;
    cliaddrlen = sizeof(cliaddr);

    client_fd = accept(server_fd, (struct sockaddr*)&cliaddr, &cliaddrlen);
    assert(client_fd != -1);

    printf("Accept new connection:%s %d\n", inet_ntoa(cliaddr.sin_addr),\
            cliaddr.sin_port);
    printf("Client_fd is:%d\n", client_fd);

    client_event = qevent_new();
    qevent_set(client_event, client_fd, QEVENT_READ, recv_msg, client_event);
    qevent_bus_attach(event->m_bus, client_event);
    qevent_bus_attach(event->m_bus, event);
}

int main()
{
    int listen_fd;
    int yes, reuse;
    qevent *listen_event;
    qevent_bus *bus;
    struct sockaddr_in serv_addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd != -1);//linux return -1, when there is error

    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        assert(0);
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
        assert(0);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(PORT);
    if(bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        assert(0);

    listen(listen_fd, LISTEN);

    bus = qevent_bus_new(NULL);
    listen_event = qevent_new();
    qevent_set(listen_event, listen_fd, QEVENT_READ, listenFunc, listen_event);

    qevent_bus_attach(bus, listen_event);

    printf("Server Waiting for new connection...\n");
    qevent_bus_run(bus);

    qevent_bus_destroy_all();

    qmemInfoDebug(0);

    return 0;
}
