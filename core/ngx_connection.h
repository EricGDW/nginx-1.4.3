
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;                 /* 监听的文件描述符 */

    struct sockaddr    *sockaddr;           /* socket地址 */
    socklen_t           socklen;            /* size of sockaddr */
    size_t              addr_text_max_len;  /* 地址转换后的最大长度 */
    ngx_str_t           addr_text;          /* 转换后的sock地址 */

    int                 type;

    /* 每个网络接口接收数据包的速率比内核处理这些包的速率快时允许送到队列的数据包的最大数目,默认值511 */
    int                 backlog;            /* 同时能处理的最大连接 */
    int                 rcvbuf;             /* 接收缓冲区大小 */
    int                 sndbuf;             /* 发送缓冲区大小 */
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;           /* 开始首次KeepAlive探测前的TCP空闭时间 */
    int                 keepintvl;          /* 两次KeepAlive探测间的时间间隔 */
    int                 keepcnt;            /* 判定断开前的KeepAlive探测次数 */
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler;

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;                   /* TODO: error_log directive */

    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;               /* 上一个listen socket */
    ngx_connection_t   *connection;             /* 监听的连接 */

    /* 状态标志 */
    unsigned            open:1;                 /* sock是否已打开 */
    unsigned            remain:1;               /* sock是否保持 */
    unsigned            ignore:1;               /* 是否忽略监听 */

    unsigned            bound:1;                /* already bound */
    unsigned            inherited:1;            /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;               /* shared between threads or processes */
    unsigned            addr_ntop:1;            /* 地址是否已转换 */

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;             /* 只用ipv6地址 */
#endif
    unsigned            keepalive:2;            /* 监听是否保持 */

#if (NGX_HAVE_DEFERRED_ACCEPT)                  /* 操作系统TCP层支持TCP_DEFER_ACCEPT */
    unsigned            deferred_accept:1;      /* 是否延迟accept */
    unsigned            delete_deferred:1;      /* 删除延迟accept */
    unsigned            add_deferred:1;         /* 添加延迟accept */
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;          /* socket输入过滤表 */
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;

/* 是否使用Nagle算法 */
typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;

/* 允许或禁止使用FreeBSD上的TCP_NOPUSH,或者Linux上的TCP_CORK套接字选项 */
typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01


struct ngx_connection_s {
    void               *data;
    ngx_event_t        *read;           /* 读事件*/
    ngx_event_t        *write;          /* 写事件*/

    ngx_socket_t        fd;             /* 连接的文件描述符 */

    ngx_recv_pt         recv;           /* 接收数据函数指针 */
    ngx_send_pt         send;           /* 发送数据函数指针 */
    ngx_recv_chain_pt   recv_chain;     /* 接收数据的chain函数指针 */
    ngx_send_chain_pt   send_chain;     /* 发送数据的chain函数指针 */

    ngx_listening_t    *listening;      /* 连接上监听的socket */

    off_t               sent;

    ngx_log_t          *log;

    ngx_pool_t         *pool;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    ngx_str_t           addr_text;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr; /* 本地地址 */

    ngx_buf_t          *buffer;

    ngx_queue_t         queue;

    ngx_atomic_uint_t   number;

    ngx_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            unexpected_eof:1;
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;


    unsigned            sendfile:1;      /* 是否发送文件 */
    unsigned            sndlowat:1;      /* 是否指定套接口的发送低潮限度 */
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            aio_sendfile:1;
    ngx_buf_t          *busy_sendfile;
#endif

#if (NGX_THREADS)
    ngx_atomic_t        lock;
#endif
};

/* 添加监听地址到cycle->listening，返回一个ngx_listening_t */
ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
/* 开始监听，调用bind()和listen() */
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
