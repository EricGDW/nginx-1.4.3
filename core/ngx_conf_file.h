
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CONF_FILE_H_INCLUDED_
#define _NGX_HTTP_CONF_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 *        AAAA  number of arguments
 *      FF      command flags
 *    TT        command type, i.e. HTTP "location" or "server" command
 */

#define NGX_CONF_NOARGS      0x00000001         /* command without args */
#define NGX_CONF_TAKE1       0x00000002         /* command with one arg */
#define NGX_CONF_TAKE2       0x00000004         /* command with two args */
#define NGX_CONF_TAKE3       0x00000008         /* command with ... args */
#define NGX_CONF_TAKE4       0x00000010
#define NGX_CONF_TAKE5       0x00000020
#define NGX_CONF_TAKE6       0x00000040
#define NGX_CONF_TAKE7       0x00000080

#define NGX_CONF_MAX_ARGS    8

#define NGX_CONF_TAKE12      (NGX_CONF_TAKE1|NGX_CONF_TAKE2)
#define NGX_CONF_TAKE13      (NGX_CONF_TAKE1|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE23      (NGX_CONF_TAKE2|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE123     (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3)
#define NGX_CONF_TAKE1234    (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3   \
                              |NGX_CONF_TAKE4)

#define NGX_CONF_ARGS_NUMBER 0x000000ff
#define NGX_CONF_BLOCK       0x00000100     /* 块域 后面跟{…} 例如events{…} */
#define NGX_CONF_FLAG        0x00000200     /* 指令读入1个布尔型数据  */
#define NGX_CONF_ANY         0x00000400     /* 标识该配置项为布尔类型值 i.e. "on" or "off" */
#define NGX_CONF_1MORE       0x00000800     /* 指令至少一个参数 */
#define NGX_CONF_2MORE       0x00001000
#define NGX_CONF_MULTI       0x00000000     /* compatibility 未使用*/

#define NGX_DIRECT_CONF      0x00010000     /* 直接存取CONF,配置命令包含项指不包含二级模块指令 */

#define NGX_MAIN_CONF        0x01000000     /* 指最顶层的指令 */
#define NGX_ANY_CONF         0x0F000000     /* include指令 */



#define NGX_CONF_UNSET       -1
#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
#define NGX_CONF_UNSET_PTR   (void *) -1
#define NGX_CONF_UNSET_SIZE  (size_t) -1
#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1


#define NGX_CONF_OK          NULL
#define NGX_CONF_ERROR       (void *) -1

#define NGX_CONF_BLOCK_START 1
#define NGX_CONF_BLOCK_DONE  2
#define NGX_CONF_FILE_DONE   3

#define NGX_CORE_MODULE      0x45524F43  /* "CORE" 十六进制的数就是其类型对应的ASCII码 */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" 十六进制的数就是其类型对应的ASCII码 */


#define NGX_MAX_CONF_ERRSTR  1024


struct ngx_command_s {
    ngx_str_t             name;                                                     /* 指令名称的字符串，不可以包含空格 */
    ngx_uint_t            type;                                                     /* 32bit的无符号整形，前16bit用于标识位置，后16bit用于标识参数 */
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);     /* 从配置文件中把该指令的参数(存放在ngx_conf_t中)转换为合适的数据类型，并将转换后的值保存到模块的配置结构体中(viod *conf) */
    ngx_uint_t            conf;                                                     /* 该字段含义为此指令配置文件的位置（e.g. NGX_HTTP_SRV_CONF_OFFSET -- 说明此指令的配置文件在srv_conf中） */
    ngx_uint_t            offset;                                                   /* 解析出来的配置项值所存放的地址 */
    void                 *post;                                                     /* 模块读配置的时候需要的一些零碎变量 */
};

#define ngx_null_command  { ngx_null_string, 0, NULL, 0, 0, NULL }      /* 空命令 */


struct ngx_open_file_s {
    ngx_fd_t              fd;                                               /* 文件描述符，一个int值 */
    ngx_str_t             name;                                             /* 文件名 */

    void                (*flush)(ngx_open_file_t *file, ngx_log_t *log);    /* 强制写入数据函数*/
    void                 *data;                                             /* 文件数据指针 */
};


#define NGX_MODULE_V1          0, 0, 0, 0, 0, 0, 1          /* 该宏用来初始化前7个字段 */
#define NGX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0       /* 该宏用来初始化最后8个字段 */

/* 模块定义 */

struct ngx_module_s {
    ngx_uint_t            ctx_index;                        /* 模块在上下文中的索引，可以理解为分类模块计数器 */
    ngx_uint_t            index;                            /* 模块在所有模块ngx_modules[]数组中的索引，可以理解为模块计数器 */

    /* 以下四字段留作备用，用于将来扩展 */

    ngx_uint_t            spare0;
    ngx_uint_t            spare1;
    ngx_uint_t            spare2;
    ngx_uint_t            spare3;

    ngx_uint_t            version;                          /* 版本 */

    void                 *ctx;                              /* 该模块的上下文，每个种类的模块有不同的上下文  */
    ngx_command_t        *commands;                         /* 该模块的命令集，指向一个ngx_command_t结构数组   */
    ngx_uint_t            type;                             /* 模块的种类，为core/conf/event/http/mail中的一种 */

    ngx_int_t           (*init_master)(ngx_log_t *log);     /* 初始化master */

    ngx_int_t           (*init_module)(ngx_cycle_t *cycle); /* 初始化模块 */

    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);/* 初始化工作 */
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle); /* 初始化线程 */
    void                (*exit_thread)(ngx_cycle_t *cycle); /* 退出线程 */
    void                (*exit_process)(ngx_cycle_t *cycle);/* 退出进程 */

    void                (*exit_master)(ngx_cycle_t *cycle); /* 退出master */

    /* 以下八字段留作备用，用于将来扩展 */

    uintptr_t             spare_hook0;
    uintptr_t             spare_hook1;
    uintptr_t             spare_hook2;
    uintptr_t             spare_hook3;
    uintptr_t             spare_hook4;
    uintptr_t             spare_hook5;
    uintptr_t             spare_hook6;
    uintptr_t             spare_hook7;
};

/* 核心模块定义 */

typedef struct {
    ngx_str_t             name;                                         /* 核心模块名 */
    void               *(*create_conf)(ngx_cycle_t *cycle);             /* 核心模块创建并初始化conf的函数指针 */
    char               *(*init_conf)(ngx_cycle_t *cycle, void *conf);   /* 核心模块名初始化函数指针 */
} ngx_core_module_t;

/* 配置文件定义 */

typedef struct {
    ngx_file_t            file;     /* 配置文件对应的文件 */
    ngx_buf_t            *buffer;   /* 配置文件缓存 */
    ngx_uint_t            line;     /* 某行 */
} ngx_conf_file_t;


typedef char *(*ngx_conf_handler_pt)(ngx_conf_t *cf,
    ngx_command_t *dummy, void *conf);

/* 配置结构定义 */

struct ngx_conf_s {
    char                 *name;         /* 名称 */
    ngx_array_t          *args;         /* 参数 */

    ngx_cycle_t          *cycle;        /* cycle变量 */
    ngx_pool_t           *pool;         /* 内存池 */
    ngx_pool_t           *temp_pool;    /* 临时内存池 */
    ngx_conf_file_t      *conf_file;    /* 配置文件 */
    ngx_log_t            *log;          /* 日志 */

    void                 *ctx;          /* 当前配置的上下文 */
    ngx_uint_t            module_type;  /* 模块类型 */
    ngx_uint_t            cmd_type;     /* 命令作用域范围 (e.g. NGX_HTTP_MAIN_CONF、NGX_HTTP_LOC_CONF等) */

    ngx_conf_handler_pt   handler;      /* 模块定义的handler */
    char                 *handler_conf; /* 自定义的handler */
};


typedef char *(*ngx_conf_post_handler_pt) (ngx_conf_t *cf,
    void *data, void *conf);

typedef struct {
    ngx_conf_post_handler_pt  post_handler;
} ngx_conf_post_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    char                     *old_name;
    char                     *new_name;
} ngx_conf_deprecated_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    ngx_int_t                 low;
    ngx_int_t                 high;
} ngx_conf_num_bounds_t;


typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                value;
} ngx_conf_enum_t;


#define NGX_CONF_BITMASK_SET  1

typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                mask;
} ngx_conf_bitmask_t;



char * ngx_conf_deprecated(ngx_conf_t *cf, void *post, void *data);
char *ngx_conf_check_num_bounds(ngx_conf_t *cf, void *post, void *data);


#define ngx_get_conf(conf_ctx, module)  conf_ctx[module.index]



#define ngx_conf_init_value(conf, default)                                   \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = default;                                                      \
    }

#define ngx_conf_init_ptr_value(conf, default)                               \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = default;                                                      \
    }

#define ngx_conf_init_uint_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_size_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_init_msec_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = default;                                                      \
    }

#define ngx_conf_merge_value(conf, prev, default)                            \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_ptr_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = (prev == NGX_CONF_UNSET_PTR) ? default : prev;                \
    }

#define ngx_conf_merge_uint_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = (prev == NGX_CONF_UNSET_UINT) ? default : prev;               \
    }

#define ngx_conf_merge_msec_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = (prev == NGX_CONF_UNSET_MSEC) ? default : prev;               \
    }

#define ngx_conf_merge_sec_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_size_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = (prev == NGX_CONF_UNSET_SIZE) ? default : prev;               \
    }

#define ngx_conf_merge_off_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_str_value(conf, prev, default)                        \
    if (conf.data == NULL) {                                                 \
        if (prev.data) {                                                     \
            conf.len = prev.len;                                             \
            conf.data = prev.data;                                           \
        } else {                                                             \
            conf.len = sizeof(default) - 1;                                  \
            conf.data = (u_char *) default;                                  \
        }                                                                    \
    }

#define ngx_conf_merge_bufs_value(conf, prev, default_num, default_size)     \
    if (conf.num == 0) {                                                     \
        if (prev.num) {                                                      \
            conf.num = prev.num;                                             \
            conf.size = prev.size;                                           \
        } else {                                                             \
            conf.num = default_num;                                          \
            conf.size = default_size;                                        \
        }                                                                    \
    }

#define ngx_conf_merge_bitmask_value(conf, prev, default)                    \
    if (conf == 0) {                                                         \
        conf = (prev == 0) ? default : prev;                                 \
    }


char *ngx_conf_param(ngx_conf_t *cf);
char *ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename);
char *ngx_conf_include(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


ngx_int_t ngx_conf_full_name(ngx_cycle_t *cycle, ngx_str_t *name,
    ngx_uint_t conf_prefix);
ngx_open_file_t *ngx_conf_open_file(ngx_cycle_t *cycle, ngx_str_t *name);
void ngx_cdecl ngx_conf_log_error(ngx_uint_t level, ngx_conf_t *cf,
    ngx_err_t err, const char *fmt, ...);


char *ngx_conf_set_flag_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_str_array_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_conf_set_keyval_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_num_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_size_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_off_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_msec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_sec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bufs_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_enum_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_set_bitmask_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_uint_t     ngx_max_module;
extern ngx_module_t  *ngx_modules[];


#endif /* _NGX_HTTP_CONF_FILE_H_INCLUDED_ */
