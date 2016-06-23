#ifndef WORKER_H
#define WORKER_H


#endif

#include <event.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


class worker
{
public:
    worker(int fd);
    ~worker();
    bool init();
    bool run();
    static void dispath(int fd, short event, void* arg);
    static void read_buff(struct bufferevent *bev, void *arg);
    static void read_err(struct bufferevent *bev, short event, void *arg);
    static void *thread(void * arg);
    bool add_client(int fd);

private:

    struct event_base 	*base;
    struct event 		*event;
    int pfd;
    pthread_t pthid;
    pthread_attr_t pthattr;

};
