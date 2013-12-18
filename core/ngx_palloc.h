
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)                     /* 允许从内存池分配的最大值 */

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)                            /* 内存池默认大小 */

#define NGX_POOL_ALIGNMENT       16                                     /* 内存池内存对齐大小 */
#define NGX_MIN_POOL_SIZE                                           \   /* 内存池最小值 */
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),  \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;  /* 函数指针，指向一个可以释放data所对应资源的函数 */
    void                 *data;     /* 释放的资源 */
    ngx_pool_cleanup_t   *next;     /* 指向该链表中的下一个元素 */
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

/* 用来指向大块内存 */
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;     /* 链表指针 */
    void                 *alloc;    /* 指向分配的大块内存 */
};

/* 内存池的数据块信息 */
typedef struct {
    u_char               *last;     /* 当前内存池分配到此处，即下一次分配从此处开始 */
    u_char               *end;      /* 内存池结束位置 */
    ngx_pool_t           *next;     /* 链接到下一个内存池 */
    ngx_uint_t            failed;   /* 统计该内存池不能满足分配请求的次数，超过4次则该数据块下次不再用来分配 */
} ngx_pool_data_t;

/* 内存池的头部信息 */
struct ngx_pool_s {
    ngx_pool_data_t       d;        /* 内存池的数据块 */
    size_t                max;      /* 内存池数据块的最大值 */
    ngx_pool_t           *current;  /* 指向内存池可用数据快链表头,下次分配从该数据块开始检测 */
    ngx_chain_t          *chain;    /* 该指针挂接一个ngx_chain_t结构 */
    ngx_pool_large_t     *large;    /* 大块内存链表，即分配空间超过max的内存 */
    ngx_pool_cleanup_t   *cleanup;  /* 释放内存池的callback及释放的资源 */
    ngx_log_t            *log;      /* 日志信息 */
};

/* 释放的文件 */
typedef struct {
    ngx_fd_t              fd;       /* 文件描述符 */
    u_char               *name;     /* 文件名 */
    ngx_log_t            *log;      /* 日志 */
} ngx_pool_cleanup_file_t;


void *ngx_alloc(size_t size, ngx_log_t *log);   /* 分配指定大小的内存 */
void *ngx_calloc(size_t size, ngx_log_t *log);  /* 分配指定大小的内存并清零 */

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);   /* 创建pool */
void ngx_destroy_pool(ngx_pool_t *pool);                    /* 销毁pool */
void ngx_reset_pool(ngx_pool_t *pool);                      /* 重置内存池 */

void *ngx_palloc(ngx_pool_t *pool, size_t size);                        /* 在内存池分配指定大小的内存，按NGX_ALIGNMENT对齐 */
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);                       /* 在内存池分配指定大小的内存,未对齐 */
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);                       /* 在内存池分配指定大小的内存并清零 */
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);   /* 在内存池按照指定对齐大小alignment来申请一块大小为size的内存 */
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);                         /* 释放内存池中large字段管理大块的指定大快内存P */


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);   /* 添加一个cleanup资源 */
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);             /* 清理p->cleanup链表上的所有挂载文件fd */
void ngx_pool_cleanup_file(void *data);                                 /* 封装ngx_close_file, 关闭data */
void ngx_pool_delete_file(void *data);                                  /* 删除data，先ngx_delete_file后ngx_close_file */


#endif /* _NGX_PALLOC_H_INCLUDED_ */
