
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;     /* e.g. 在函数 ngx_http_file_cache_set_slot（） 被设置成 ngx_http_file_cache_t */
    ngx_shm_t                 shm;      /* 共享内存属性 */
    ngx_shm_zone_init_pt      init;     /* e.g. 在函数 ngx_http_file_cache_set_slot（） 被设置成  ngx_http_file_cache_init() */
    void                     *tag;      /* 使用共享内存的模块名称（内存地址） */
};


struct ngx_cycle_s {
    void                  ****conf_ctx;                     /* 模块配置结构指针数组(含所有模块) */
    ngx_pool_t               *pool;                         /* 内存池 */

    ngx_log_t                *log;                          /* 日志指针 */
    ngx_log_t                 new_log;

    ngx_connection_t        **files;                        /* 连接文件，存放socket和connection之间的关系 */
    ngx_connection_t         *free_connections;             /* 指向空闲连接的链表 */
    ngx_uint_t                free_connection_n;            /* 空闲连接的个数 */

    ngx_queue_t               reusable_connections_queue;   /* 复用连接的队列 */

    ngx_array_t               listening;                    /* 监听数组,元素结构为ngx_listening_t */
    ngx_array_t               paths;                        /* 路径数组,元素结构为ngx_path_t */
    ngx_list_t                open_files;                   /* 打开文件链表,元素结构为ngx_open_file_t */
    ngx_list_t                shared_memory;                /* 共享内存链表,元素结构为ngx_shm_zone_t */

    ngx_uint_t                connection_n;                 /* 预先创建的connection数目 */
    ngx_uint_t                files_n;                      /* 打开文件数 */

    ngx_connection_t         *connections;                  /* 连接事件的双向链表 */
    ngx_event_t              *read_events;                  /* 读事件的双向链表*/
    ngx_event_t              *write_events;                 /* 写事件的双向链表 */

    ngx_cycle_t              *old_cycle;                    /* old cycle指针 */

    ngx_str_t                 conf_file;                    /* 配置文件 */
    ngx_str_t                 conf_param;                   /* 配置参数 */
    ngx_str_t                 conf_prefix;                  /* 配置前缀，配置文件路径 */
    ngx_str_t                 prefix;                       /* 前缀，默认为安装路径 */
    ngx_str_t                 lock_file;                    /* 锁文件,在init_conf时设置。使用连接互斥锁进行顺序的accept()系统调用默认是“logs/nginx.lock.accept” */
    ngx_str_t                 hostname;                     /* 主机名 */
};


typedef struct {
     ngx_flag_t               daemon;               /* 是否以守护进程启动，默认为on，调试时会用off */
     ngx_flag_t               master;               /* 指令master_process指定此字段，on = 1、off = 0 */

     ngx_msec_t               timer_resolution;     /* 指令"timer_resolution"设置的时间，调用gettimeofday的间隔，可以减少调用次数 */

     ngx_int_t                worker_processes;     /* 工作进程数 */
     ngx_int_t                debug_points;         /* 在调试器内设置断点 */

     ngx_int_t                rlimit_nofile;        /* 此字段在worker进程初始化时设置到内核。指令worker_rlimit_nofile 进程能够打开的最多文件描述符数 */
     ngx_int_t                rlimit_sigpending;    /* 调用进程中真实用户队列的信号数量限制 */
     off_t                    rlimit_core;          /* 允许每个进程核心文件的最大值 */

     int                      priority;             /* 优先级 */

     ngx_uint_t               cpu_affinity_n;       /* cpu绑定的个数 */
     uint64_t                *cpu_affinity;         /* 绑定的cpu */

     char                    *username;             /* 指令 "user" 设置的用户名 */
     ngx_uid_t                user;                 /* uid */
     ngx_gid_t                group;                /* gid */

     ngx_str_t                working_directory;    /* 工作的目录 */
     ngx_str_t                lock_file;            /* 指令lock_file指定锁文件名，未指定时使用默认“logs/nginx.lock” */

     ngx_str_t                pid;                  /* 进程号文件路径名,如果没有指定存放pid的文件路径则使用默认“logs/nginx.pid”路径 */
     ngx_str_t                oldpid;               /* 老的进程号文件路径名 */

     ngx_array_t              env;                  /* 运行上下文 */
     char                   **environment;          /* 环境变量 */

#if (NGX_THREADS)
     ngx_int_t                worker_threads;       /* 工作线程数 */
     size_t                   thread_stack_size;    /* 线程栈大小 */
#endif

} ngx_core_conf_t;


typedef struct {
     ngx_pool_t              *pool;   /* pcre's malloc() pool */
} ngx_core_tls_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
uint64_t ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_quiet_mode;
#if (NGX_THREADS)
extern ngx_tls_key_t          ngx_core_tls_key;
#endif


#endif /* _NGX_CYCLE_H_INCLUDED_ */
