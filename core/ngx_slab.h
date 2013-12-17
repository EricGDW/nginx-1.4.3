
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*
 * 用于表示内存大小分级的ngx_slab_pool_t称为slot。整个分级区域称为slots数组。
 * 用于实际内存分配的pagesize 大小的内存块称为slab。整个可用内存块区域称为slab数组。
 * 用于管理slab的ngx_slab_pool_t称为slab管理结构体。整个管理结构体区域称为slab管理结构体数组。
 * 从slab从切分出来的用于完于内存请求的内存块称为chunk。用于跟踪整个slab中chunk的使用情况的结构称为bitmap。
 * slot的分级是按照2的幂次方来的，而用来计算2的幂次方时，移位操作又是最快的。
 * 我们将每个slot对应的幂值称为shift，那么这个slot能够分配的chunk大小就是1 << shift。
 * 这个slot申请来的slab可以切分出1 << (ngx_pagesize_shift - shift)个chunk。
 */

typedef struct ngx_slab_page_s  ngx_slab_page_t;

struct ngx_slab_page_s {
    uintptr_t         slab;  /* 不同场景下用于：存储bitmap、shift值或者后续连续的空闲slab个数 */
    ngx_slab_page_t  *next;
    uintptr_t         prev;  /* 双链表的前向指针，同时存储slab属性信息 */
};


typedef struct {    /* 内存缓存池 */
    ngx_shmtx_sh_t    lock;         /* mutex的锁 */

    size_t            min_size;     /* 内存缓存obj最小的大小，一般是1个byte */
    size_t            min_shift;    /* 最小slot的chunk对应的幂值，默认为3*/

    ngx_slab_page_t  *pages;        /* slab的管理结构体数组首地址 */
    ngx_slab_page_t   free;         /* 可用slab page的开头,从free.next开始分配 */

    u_char           *start;        /* 实际slab存储对象空间的开头  */
    u_char           *end;          /* 整个slab pool空间的结尾 */

    ngx_shmtx_t       mutex;        /* 互斥锁 */

    u_char           *log_ctx;
    u_char            zero;

    void             *data;         /* 用户数据 */
    void             *addr;         /* 指向ngx_slab_pool_t的开头 */
} ngx_slab_pool_t;


void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);


#endif /* _NGX_SLAB_H_INCLUDED_ */
