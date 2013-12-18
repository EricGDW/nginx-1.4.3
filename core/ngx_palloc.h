
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
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)                     /* ������ڴ�ط�������ֵ */

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)                            /* �ڴ��Ĭ�ϴ�С */

#define NGX_POOL_ALIGNMENT       16                                     /* �ڴ���ڴ�����С */
#define NGX_MIN_POOL_SIZE                                           \   /* �ڴ����Сֵ */
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),  \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;  /* ����ָ�룬ָ��һ�������ͷ�data����Ӧ��Դ�ĺ��� */
    void                 *data;     /* �ͷŵ���Դ */
    ngx_pool_cleanup_t   *next;     /* ָ��������е���һ��Ԫ�� */
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

/* ����ָ�����ڴ� */
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;     /* ����ָ�� */
    void                 *alloc;    /* ָ�����Ĵ���ڴ� */
};

/* �ڴ�ص����ݿ���Ϣ */
typedef struct {
    u_char               *last;     /* ��ǰ�ڴ�ط��䵽�˴�������һ�η���Ӵ˴���ʼ */
    u_char               *end;      /* �ڴ�ؽ���λ�� */
    ngx_pool_t           *next;     /* ���ӵ���һ���ڴ�� */
    ngx_uint_t            failed;   /* ͳ�Ƹ��ڴ�ز��������������Ĵ���������4��������ݿ��´β����������� */
} ngx_pool_data_t;

/* �ڴ�ص�ͷ����Ϣ */
struct ngx_pool_s {
    ngx_pool_data_t       d;        /* �ڴ�ص����ݿ� */
    size_t                max;      /* �ڴ�����ݿ�����ֵ */
    ngx_pool_t           *current;  /* ָ���ڴ�ؿ������ݿ�����ͷ,�´η���Ӹ����ݿ鿪ʼ��� */
    ngx_chain_t          *chain;    /* ��ָ��ҽ�һ��ngx_chain_t�ṹ */
    ngx_pool_large_t     *large;    /* ����ڴ�����������ռ䳬��max���ڴ� */
    ngx_pool_cleanup_t   *cleanup;  /* �ͷ��ڴ�ص�callback���ͷŵ���Դ */
    ngx_log_t            *log;      /* ��־��Ϣ */
};

/* �ͷŵ��ļ� */
typedef struct {
    ngx_fd_t              fd;       /* �ļ������� */
    u_char               *name;     /* �ļ��� */
    ngx_log_t            *log;      /* ��־ */
} ngx_pool_cleanup_file_t;


void *ngx_alloc(size_t size, ngx_log_t *log);   /* ����ָ����С���ڴ� */
void *ngx_calloc(size_t size, ngx_log_t *log);  /* ����ָ����С���ڴ沢���� */

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);   /* ����pool */
void ngx_destroy_pool(ngx_pool_t *pool);                    /* ����pool */
void ngx_reset_pool(ngx_pool_t *pool);                      /* �����ڴ�� */

void *ngx_palloc(ngx_pool_t *pool, size_t size);                        /* ���ڴ�ط���ָ����С���ڴ棬��NGX_ALIGNMENT���� */
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);                       /* ���ڴ�ط���ָ����С���ڴ�,δ���� */
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);                       /* ���ڴ�ط���ָ����С���ڴ沢���� */
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);   /* ���ڴ�ذ���ָ�������Сalignment������һ���СΪsize���ڴ� */
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);                         /* �ͷ��ڴ����large�ֶι������ָ������ڴ�P */


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);   /* ���һ��cleanup��Դ */
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);             /* ����p->cleanup�����ϵ����й����ļ�fd */
void ngx_pool_cleanup_file(void *data);                                 /* ��װngx_close_file, �ر�data */
void ngx_pool_delete_file(void *data);                                  /* ɾ��data����ngx_delete_file��ngx_close_file */


#endif /* _NGX_PALLOC_H_INCLUDED_ */
