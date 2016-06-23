#include "worker.h"


typedef union
{
    int fd;
    char buff[4];
}INFO;

 bool worker::init()
 {
     base = event_base_new();
     assert(base != NULL);

     //每个worker线程的读管道监听来自master的通知
     event = event_new(base, pfd, EV_READ|EV_PERSIST, dispath, (void*)this);
     assert(event != NULL);

     if (event_add(event, NULL) == -1)
     {
        printf("%s","线程增加监听事件错误\n");
         return false;
     }

     int ret;

     if ((ret = pthread_create(&pthid, &pthattr, thread, this)) )
     {
         printf("创建worker线程失败 %d\n",ret);
     }
     return true;
 }

void worker::dispath(int fd, short event, void* arg)
 {
     INFO clnt;
     worker* wker = static_cast<worker*>(arg);

     if (read(fd, clnt.buff, sizeof(clnt)) != 4)
     {
         printf("从管道中读取书记为空\n");
         return;
     }

     printf("线程管道接收到信息 %d\n",clnt.fd);

     if (!wker->add_client(clnt.fd))
     {
         printf("%s","线程添加客户端失败\n");
         return ;
     }

     return ;
 }


bool worker::add_client(int fd)
 {
     evutil_make_socket_nonblocking(fd);

    struct bufferevent *buffevt = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buffevt)
    {
        printf("添加客户端事件失败!\n");
        return false;
    }

    bufferevent_setcb(buffevt, read_buff, NULL, read_err, (void*) this);

    //利用客户端心跳超时机制处理半开连接
    struct timeval heartbeat;
    heartbeat.tv_sec = 10000;

    //bufferevent_set_timeouts(buffevt, &heartbeat, NULL);

    bufferevent_enable(buffevt, EV_READ | EV_PERSIST);

    return true;
 }

void worker::read_buff(struct bufferevent *bev, void *arg)
{
    int len = 0;
    char buff[1024];
    if ((len = bufferevent_read(bev, buff, sizeof(buff))) > 0)
    {
        printf("线程接收客户端信息%s\n",buff);
    }
    memset(buff,0,sizeof(buff));
    return ;
}
/*
{
    printf("called readcb!\n");
    struct evbuffer *input, *output;
    char *request_line;
    size_t len;
    input = bufferevent_get_input(bev);//其实就是取出bufferevent中的input
    output = bufferevent_get_output(bev);//其实就是取出bufferevent中的output

    size_t input_len = evbuffer_get_length(input);
    printf("input_len: %d\n", input_len);
    size_t output_len = evbuffer_get_length(output);
    printf("output_len: %d\n", output_len);

    while(1)
    {
        request_line = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);//从evbuffer前面取出一行，用一个新分配的空字符结束的字符串返回这一行,EVBUFFER_EOL_CRLF表示行尾是一个可选的回车，后随一个换行符
        if(NULL == request_line)
        {
            printf("The first line has not arrived yet.\n");
            free(request_line);//之所以要进行free是因为 line = mm_malloc(n_to_copy+1))，在这里进行了malloc
            break;
        }
        else
        {
            printf("Get one line date: %s\n", request_line);
            if(strstr(request_line, "over") != NULL)//用于判断是不是一条消息的结束
            {
                char *response = "Response ok! Hello Client!\r\n";
                evbuffer_add(output, response, strlen(response));//Adds data to an event buffer
                printf("服务端接收一条数据完成，回复客户端一条消息: %s\n", response);
                free(request_line);
                break;
            }
        }
        free(request_line);
    }

    size_t input_len1 = evbuffer_get_length(input);
    printf("input_len1: %d\n", input_len1);
    size_t output_len1 = evbuffer_get_length(output);
    printf("output_len1: %d\n\n", output_len1);
}
*/


void worker::read_err(struct bufferevent *bev, short event, void *arg)
{

    if (event & BEV_EVENT_TIMEOUT)
    {
        printf("客户端链接超时\n");
    }
    else if (event & BEV_EVENT_EOF)
    {
    }
    else if (event & BEV_EVENT_ERROR)
    {
    }
    printf("客户端链接断开\n");
    bufferevent_free(bev);
    return ;
}


void* worker::thread(void * arg)
{
    worker* wker = static_cast<worker*>(arg);

    int ret = event_base_dispatch(wker->base);
    if (-1 == ret)
    {
        printf("线程轮循失败\n");
        return false;
    }
    return NULL;
}


worker::worker(int fd)
{
    pfd = fd;
}

worker::~worker()
{}
