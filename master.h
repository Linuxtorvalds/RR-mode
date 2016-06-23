#ifndef MASTER_H

#define MASTER_H

#endif

#include <memory>
#include <string>
#include <event.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>



#include "worker.h"


class master
{
public:

    master();
    ~master();

public:

    bool init();
    bool run();
    static void laccept(evutil_socket_t socket, short event, void* arg);
    void dispatch(int client);

private:

    int fd;
    int num;
    struct event_base   *base;
    struct event    *event;
    std::vector<int>  pipes;

};
