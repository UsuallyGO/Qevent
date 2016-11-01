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

int main()
{
    int sock_fd;
    int res, ran;
    struct sockaddr_in serv_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr);

    res = connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    assert(res >= 0);
    printf("Server has connected, socket fd:%d...\n", sock_fd);

    while(1)
    { 
        ran = rand()%5 + 1;
        printf("sleep for %d seconds...\n", ran);
        sleep(ran);

        printf("Write message to server...\n");
        res = send(sock_fd, "Hello server\n", 32, 0);
    }

    close(sock_fd);
    return 0;
}

