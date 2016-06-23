#include "master.h"

//using namespace std;


master::master()
{}
master::~master()
{}

bool master::init()
{

    for (int i = 0; i < 2; i++)
    {
        int pfd[2];

        if (pipe(pfd) != 0)
        {
            printf("%s","创建线程管道错误\n");
            return false;
        }
        pipes.push_back(pfd[1]);

        worker* wker = new worker(pfd[0]);

        if (!wker)
        {
            printf("%s","申请worker空间失败\n");
            return false;
        }
        wker->init();
        /*
        if (! wker->init())
        {
            printf("%s","创建工作线程失败\n");
            return false;
        }
       */
    }

    return true;
}



void master::laccept(evutil_socket_t socket, short event, void* arg)
 {
    int client;
    master *mster = static_cast<master*> (arg);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    client = accept(socket, (struct sockaddr *) &client_addr, &addr_len);
    if (-1 == client)
        return;

    printf("接收到客户端链接 %d\n",client);    

    mster->dispatch(client);

    return;
 }

//分发到各个线程的管道，通知woker读取
void master::dispatch(int client)
{

    int id;
    id = (num++) % 2;
    int pfd = pipes.at(id);
    if ( pfd > 0 )
    {
        int len;
        len =  write(pfd, (void*)&client, sizeof(client));
        if (len< 0)
        {
            printf("写入管道错误 %d %d\n",pfd, len);
        }
        printf("写入管道成功 %d %d %d\n", client, id, len);
        return ;
    }
    printf("主线程管道为空\n");

    return ;
}
bool master::run()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)  return false;

    int flags = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &flags, sizeof(flags)) != 0)
    {
        close(fd);
        return false;
    }
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *) &flags, sizeof(flags)) != 0)
    {
        close(fd);
        return false;
    }
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(80);

    if (bind(fd, (const sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        close(fd);
        return false;
    }

    if(listen(fd, 10) != 0)
    {
        close(fd);
        return false;
    }

    evutil_make_socket_nonblocking(fd);

    //初始化base
    base = event_base_new();
    assert(base != NULL);

    //设置事件
    event = event_new(base, fd, EV_READ|EV_PERSIST, laccept, (void*)this);
    assert(event != NULL);

    //设置为base事件
    event_base_set(base, event);
    //添加事件
    if(event_add(event, NULL) == -1)
    {
        return false;
    }
    //事件循环
    int ret = event_base_dispatch(base);
    if (-1 == ret)
    {
        exit(1);
    }
    return true;
}

