
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILE_H_INCLUDED_
#define _NGX_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_file_s {
    ngx_fd_t                   fd;          /* 文件描述符 */
    ngx_str_t                  name;        /* 文件名 */
    ngx_file_info_t            info;        /* 文件元数据 */

    off_t                      offset;      /* 已写入文件中的数据偏移量 */
    off_t                      sys_offset;  /* 文件描述符 */

    ngx_log_t                 *log;         /* 日志 */

#if (NGX_HAVE_FILE_AIO)
    ngx_event_aio_t           *aio;         /* 文件AIO */
#endif

    unsigned                   valid_info:1;/* 是否验证了元数据 */
    unsigned                   directio:1;  /* 是否开启了directio */
};


#define NGX_MAX_PATH_LEVEL  3


typedef time_t (*ngx_path_manager_pt) (void *data);
typedef void (*ngx_path_loader_pt) (void *data);


typedef struct {
    ngx_str_t                  name;        /* 路径的名称 */
    size_t                     len;         /* 子目录文件名称长度大小包括斜杠长度（e.g. /ABC/sub1/sub2/sub3/ --> 3+1+3+1+3+1=12 */
    size_t                     level[3];    /* 3级子目录，每个子目录的名称长度 */

    /* 对应的回调，以及回调数据 */

    ngx_path_manager_pt        manager;     /* ngx_http_file_cache_set_slot()函数中设置 */
    ngx_path_loader_pt         loader;      /* ngx_http_file_cache_set_slot()函数中设置 */
    void                      *data;        /* 设置为ngx_http_file_cache_t 在ngx_http_file_cache_set_slot()函数中设置 */

    u_char                    *conf_file;   /* 配置文件的地址指针 */
    ngx_uint_t                 line;        /* 解析的行号 */
} ngx_path_t;


typedef struct {
    ngx_str_t                  name;
    size_t                     level[3];
} ngx_path_init_t;


typedef struct {
    ngx_file_t                 file;            /* 临时文件信息 */
    off_t                      offset;          /* 已写入文件中的数据偏移量 */
    ngx_path_t                *path;            /* 临时文件存放的目录 */
    ngx_pool_t                *pool;            /* 使用的内存池 */
    char                      *warn;            /* 未初始化变量的日志级别 */

    ngx_uint_t                 access;          /* 文件的访问权限 */

    unsigned                   log_level:8;     /* 日志等级 */
    unsigned                   persistent:1;    /* 说明临时文件是否一直存在于文件系统中 */
    unsigned                   clean:1;         /* 文件的清理方式（是否将文件在磁盘上删除） */
} ngx_temp_file_t;


typedef struct {
    ngx_uint_t                 access;          /* 文件的访问权限 */
    ngx_uint_t                 path_access;     /* 目录的访问权限 */
    time_t                     time;            /* 文件的最后修改时间 */
    ngx_fd_t                   fd;              /* 文件描述符 */

    unsigned                   create_path:1;   /* 当访问的文件目录不存在时，将创建目录 */
    unsigned                   delete_file:1;   /* 删除文件 */

    ngx_log_t                 *log;             /* 使用的日志 */
} ngx_ext_rename_file_t;


typedef struct {
    off_t                      size;        /* copy的大小 */
    size_t                     buf_size;    /* 缓存区大小 */

    ngx_uint_t                 access;      /* 文件的访问权限 */
    time_t                     time;        /* 文件的最后修改时间 */

    ngx_log_t                 *log;         /* 使用的日志 */
} ngx_copy_file_t;


typedef struct ngx_tree_ctx_s  ngx_tree_ctx_t;

typedef ngx_int_t (*ngx_tree_init_handler_pt) (void *ctx, void *prev);
typedef ngx_int_t (*ngx_tree_handler_pt) (ngx_tree_ctx_t *ctx, ngx_str_t *name);

struct ngx_tree_ctx_s {
    off_t                      size;                /* 文件大小 */
    off_t                      fs_size;             /* 占用的文件系统空间大小 */
    ngx_uint_t                 access;              /* access authority */
    time_t                     mtime;               /* last modyfied time */

    ngx_tree_init_handler_pt   init_handler;        /*  see ctx->alloc */
    ngx_tree_handler_pt        file_handler;        /*  file handler */
    ngx_tree_handler_pt        pre_tree_handler;    /*  handler is called before entering directory */
    ngx_tree_handler_pt        post_tree_handler;   /*  handler is called after leaving directory */
    ngx_tree_handler_pt        spec_handler;        /*  special (socket, FIFO, etc.) file handler */

    void                      *data;                /* some data structure, it may be the same on all levels, or reallocated if ctx->alloc is nonzero */
    size_t                     alloc;               /* a size of data structure that is allocated at every level and is initilialized by ctx->init_handler() */

    ngx_log_t                 *log;                 /* a log */
};


ssize_t ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain);
ngx_int_t ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path,
    ngx_pool_t *pool, ngx_uint_t persistent, ngx_uint_t clean,
    ngx_uint_t access);
void ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len);
ngx_int_t ngx_create_path(ngx_file_t *file, ngx_path_t *path);
ngx_err_t ngx_create_full_path(u_char *dir, ngx_uint_t access);
ngx_int_t ngx_add_path(ngx_conf_t *cf, ngx_path_t **slot);
ngx_int_t ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user);
ngx_int_t ngx_ext_rename_file(ngx_str_t *src, ngx_str_t *to,
    ngx_ext_rename_file_t *ext);
ngx_int_t ngx_copy_file(u_char *from, u_char *to, ngx_copy_file_t *cf);
ngx_int_t ngx_walk_tree(ngx_tree_ctx_t *ctx, ngx_str_t *tree);

ngx_atomic_uint_t ngx_next_temp_number(ngx_uint_t collision);

char *ngx_conf_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_merge_path_value(ngx_conf_t *cf, ngx_path_t **path,
    ngx_path_t *prev, ngx_path_init_t *init);
char *ngx_conf_set_access_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_atomic_t      *ngx_temp_number;
extern ngx_atomic_int_t   ngx_random_number;


#endif /* _NGX_FILE_H_INCLUDED_ */
