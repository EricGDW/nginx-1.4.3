
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_RESOLVER_H_INCLUDED_
#define _NGX_RESOLVER_H_INCLUDED_


#define NGX_RESOLVE_A         1
#define NGX_RESOLVE_CNAME     5
#define NGX_RESOLVE_PTR       12
#define NGX_RESOLVE_MX        15
#define NGX_RESOLVE_TXT       16
#define NGX_RESOLVE_DNAME     39

#define NGX_RESOLVE_FORMERR   1
#define NGX_RESOLVE_SERVFAIL  2
#define NGX_RESOLVE_NXDOMAIN  3
#define NGX_RESOLVE_NOTIMP    4
#define NGX_RESOLVE_REFUSED   5
#define NGX_RESOLVE_TIMEDOUT  NGX_ETIMEDOUT


#define NGX_NO_RESOLVER       (void *) -1

#define NGX_RESOLVER_MAX_RECURSION    50


typedef struct {
    ngx_connection_t         *connection;
    struct sockaddr          *sockaddr;
    socklen_t                 socklen;
    ngx_str_t                 server;
    ngx_log_t                 log;
} ngx_udp_connection_t;


typedef struct ngx_resolver_ctx_s  ngx_resolver_ctx_t;

typedef void (*ngx_resolver_handler_pt)(ngx_resolver_ctx_t *ctx);


typedef struct {
    ngx_rbtree_node_t         node;         /* DNS服务器（ngx_udp_connection_t）所在红黑树节点，红黑树是为了方便查找 */
    ngx_queue_t               queue;        /* DNS服务器（ngx_udp_connection_t）所在队列，队列为了方便超时管理 */

    /* PTR: resolved name, A: name to resolve */
    u_char                   *name;         /* 待解析的DNS名称 */

    u_short                   nlen;         /* 待解析的DNS名称长度 */
    u_short                   qlen;         /* 查询字符串长度 */

    u_char                   *query;        /* 查询字符串 */

    union {
        in_addr_t             addr;
        in_addr_t            *addrs;
        u_char               *cname;
    } u;                                    /* 解析后的结果，若为name解析到IP,则u为addr或addrs,若为IP解析到name,这u为cname */

    u_short                   naddrs;       /* IP数 */
    u_short                   cnlen;        /* 解析得到的name长度，即cname的长度 */

    time_t                    expire;       /* 解析超时时间 */
    time_t                    valid;        /* 有效时间 */

    ngx_resolver_ctx_t       *waiting;      /* 节点所处的上下文 */
} ngx_resolver_node_t;


typedef struct {
    /* has to be pointer because of "incomplete type" */
    ngx_event_t              *event;                /* DNS解析的事件支持 */
    void                     *dummy;                /* 假指针，参考ident */
    ngx_log_t                *log;                  /* 日志指针 */

    /* ident must be after 3 pointers */
    ngx_int_t                 ident;                /* */

    /* simple round robin DNS peers balancer */
    ngx_array_t               udp_connections;      /* 连接的dns服务器数组，数组元素类型为ngx_udp_connection_t */
    ngx_uint_t                last_connection;      /* 最后一次连接的dns服务器*/

    ngx_rbtree_t              name_rbtree;          /* 基于名称的本地缓存DNS数据,红黑树的方式来组织数据，以达到快速查找 */
    ngx_rbtree_node_t         name_sentinel;        /* 基于名称的本地缓存DNS数据红黑数的哨兵节点*/

    ngx_rbtree_t              addr_rbtree;          /* 基于地址的本地缓存DNS数据,红黑树的方式来组织数据，以达到快速查找 */
    ngx_rbtree_node_t         addr_sentinel;        /* 基于地址的本地缓存DNS数据红黑数的哨兵节点*/


    ngx_queue_t               name_resend_queue;    /* 基于名称的重发缓存DNS队列*/
    ngx_queue_t               addr_resend_queue;    /* 基于地址的重发缓存DNS队列*/

    ngx_queue_t               name_expire_queue;    /* 基于名称的超时缓存DNS队列*/
    ngx_queue_t               addr_expire_queue;    /* 基于地址的超时缓存DNS队列*/

    time_t                    resend_timeout;       /* 重新发送的超时时间 */
    time_t                    expire;               /* 超时时间 */
    time_t                    valid;                /* 缓存时间，指令 "resolver" 指定valid参数时间为秒 */

    ngx_uint_t                log_level;            /* 日志级别 */
} ngx_resolver_t;


struct ngx_resolver_ctx_s {
    ngx_resolver_ctx_t       *next;             /* 日志级别 */
    ngx_resolver_t           *resolver;         /* 日志级别 */
    ngx_udp_connection_t     *udp_connection;   /* 日志级别 */

    /* ident must be after 3 pointers */
    ngx_int_t                 ident;

    ngx_int_t                 state;
    ngx_int_t                 type;
    ngx_str_t                 name;

    ngx_uint_t                naddrs;
    in_addr_t                *addrs;
    in_addr_t                 addr;

    ngx_resolver_handler_pt   handler;
    void                     *data;
    ngx_msec_t                timeout;

    ngx_uint_t                quick;  /* unsigned  quick:1; */
    ngx_uint_t                recursion;
    ngx_event_t              *event;
};


ngx_resolver_t *ngx_resolver_create(ngx_conf_t *cf, ngx_str_t *names,
    ngx_uint_t n);
ngx_resolver_ctx_t *ngx_resolve_start(ngx_resolver_t *r,
    ngx_resolver_ctx_t *temp);
ngx_int_t ngx_resolve_name(ngx_resolver_ctx_t *ctx);
void ngx_resolve_name_done(ngx_resolver_ctx_t *ctx);
ngx_int_t ngx_resolve_addr(ngx_resolver_ctx_t *ctx);
void ngx_resolve_addr_done(ngx_resolver_ctx_t *ctx);
char *ngx_resolver_strerror(ngx_int_t err);


#endif /* _NGX_RESOLVER_H_INCLUDED_ */
