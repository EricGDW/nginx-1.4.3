
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_STRING_H_INCLUDED_
#define _NGX_STRING_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/* 定义str,类似string类型 */
typedef struct {
    size_t      len;    /* nginx自定义类型ngx_str_t长度,data指向的字符个数*/
    u_char     *data;   /* nginx自定义类型ngx_str_t具体数据 */
} ngx_str_t;

/* 定义的key-value类型，类似map */
typedef struct {
    ngx_str_t   key;
    ngx_str_t   value;
} ngx_keyval_t;

/* 定义可变变量 */
typedef struct {
    unsigned    len:28;         /* 变量长度 */

    unsigned    valid:1;        /* 变量是否有效 */
    unsigned    no_cacheable:1  /* 变量是否不缓存 */
    unsigned    not_found:1;    /* 变量是否未找到 */
    unsigned    escape:1;       /* 变量是否编码 */

    u_char     *data;           /* 变量值 */
} ngx_variable_value_t;


#define ngx_string(str)     { sizeof(str) - 1, (u_char *) str }                 /* 宏定义，从c语言的char*转换为ngx_str_t*/
#define ngx_null_string     { 0, NULL }                                         /* 空字符串 */
#define ngx_str_set(str, text)                                                  /* 宏定义，类型为ngx_str_t的变量str初始化*/                                             \
    (str)->len = sizeof(text) - 1; (str)->data = (u_char *) text
#define ngx_str_null(str)   (str)->len = 0; (str)->data = NULL                  /* str初始化为空*/


#define ngx_tolower(c)      (u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)  /* 把字符转换为小写 */
#define ngx_toupper(c)      (u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c) /* 把字符转换为大写 */

void ngx_strlow(u_char *dst, u_char *src, size_t n);                            /* 把指定数量字符串转换为小写 */


#define ngx_strncmp(s1, s2, n)  strncmp((const char *) s1, (const char *) s2, n)/* 比较指定长度的字符串 */


/* msvc and icc7 compile strcmp() to inline loop */
#define ngx_strcmp(s1, s2)  strcmp((const char *) s1, (const char *) s2)        /* 比较字符串 */


#define ngx_strstr(s1, s2)  strstr((const char *) s1, (const char *) s2)        /* 求子字符串 */
#define ngx_strlen(s)       strlen((const char *) s)                            /* 字符串长度 */

#define ngx_strchr(s1, c)   strchr((const char *) s1, (int) c)                  /* 在字符串中找到匹配的字符，返回匹配的指针 */

static ngx_inline u_char *                                                      /* 在last前从字符串中找字符c */
ngx_strlchr(u_char *p, u_char *last, u_char c)
{
    while (p < last) {

        if (*p == c) {
            return p;
        }

        p++;
    }

    return NULL;
}


/*
 * msvc and icc7 compile memset() to the inline "rep stos"
 * while ZeroMemory() and bzero() are the calls.
 * icc7 may also inline several mov's of a zeroed register for small blocks.
 */
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)                      /* 把一片内存区设置为0 */
#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)                      /* 把一片内存区设置为指定的数c */


#if (NGX_MEMCPY_LIMIT)

void *ngx_memcpy(void *dst, const void *src, size_t n);
#define ngx_cpymem(dst, src, n)   (((u_char *) ngx_memcpy(dst, src, n)) + (n))

#else

/*
 * gcc3, msvc, and icc7 compile memcpy() to the inline "rep movs".
 * gcc3 compiles memcpy(d, s, 4) to the inline "mov"es.
 * icc8 compile memcpy(d, s, 4) to the inline "mov"es or XMM moves.
 */
#define ngx_memcpy(dst, src, n)   (void) memcpy(dst, src, n)                    /* 复制内存，没有返回 */
#define ngx_cpymem(dst, src, n)   (((u_char *) memcpy(dst, src, n)) + (n))      /* 复制内存，返回复制完了dst的最后一个字符的下一个字符的指针 */

#endif


#if ( __INTEL_COMPILER >= 800 )

/*
 * the simple inline cycle copies the variable length strings up to 16
 * bytes faster than icc8 autodetecting _intel_fast_memcpy()
 */

static ngx_inline u_char *
ngx_copy(u_char *dst, u_char *src, size_t len)
{
    if (len < 17) {

        while (len) {
            *dst++ = *src++;
            len--;
        }

        return dst;

    } else {
        return ngx_cpymem(dst, src, len);
    }
}

#else

#define ngx_copy                  ngx_cpymem                                    /* 同ngx_cpymem */

#endif


#define ngx_memmove(dst, src, n)   (void) memmove(dst, src, n)                  /* 移动内存，没有返回 */
#define ngx_movemem(dst, src, n)   (((u_char *) memmove(dst, src, n)) + (n))    /* 移动内存，返回移动完了dst的最后一个字符的下一个字符的指针 */


/* msvc and icc7 compile memcmp() to the inline loop */
#define ngx_memcmp(s1, s2, n)  memcmp((const char *) s1, (const char *) s2, n)  /* 内存比较 */


u_char *ngx_cpystrn(u_char *dst, u_char *src, size_t n);                        /* 复制指定长度字符串 */
u_char *ngx_pstrdup(ngx_pool_t *pool, ngx_str_t *src);                          /* 复制字符串到pool，返回字符串的指针 */
u_char * ngx_cdecl ngx_sprintf(u_char *buf, const char *fmt, ...);              /* 各种类型的数据格式化输出到buf，最大的长度为65536 */
u_char * ngx_cdecl ngx_snprintf(u_char *buf, size_t max, const char *fmt, ...); /* 各种类型的数据格式化输出到指定长度的buf */
u_char * ngx_cdecl ngx_slprintf(u_char *buf, u_char *last, const char *fmt,     /* 各种类型的数据格式化输出到buf, last指定了buf的结束位置 */
    ...);
u_char *ngx_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);
#define ngx_vsnprintf(buf, max, fmt, args)                                   \
    ngx_vslprintf(buf, buf + (max), fmt, args)

ngx_int_t ngx_strcasecmp(u_char *s1, u_char *s2);                               /* 不分大小写比较两个字符串是否相同 */
ngx_int_t ngx_strncasecmp(u_char *s1, u_char *s2, size_t n);                    /* 指定长短不分大小写比较两个字符串是否相同 */

u_char *ngx_strnstr(u_char *s1, char *s2, size_t n);                            /* 在指定大小一个字符串中是否有子字符串 */

u_char *ngx_strstrn(u_char *s1, char *s2, size_t n);                            /* 在一个字符串中是否有子指定大小的字符串 */
u_char *ngx_strcasestrn(u_char *s1, char *s2, size_t n);                        /* 在一个字符串中是否有子指定大小的字符串，不区分大小写 */
u_char *ngx_strlcasestrn(u_char *s1, u_char *last, u_char *s2, size_t n);       /* intended to search for static substring，n must be length of the second substring - 1 */

ngx_int_t ngx_rstrncmp(u_char *s1, u_char *s2, size_t n);                       /* 从后往前比较两个字符串是否相同，返回相同的位置 */
ngx_int_t ngx_rstrncasecmp(u_char *s1, u_char *s2, size_t n);                   /* 从后往前比较两个字符串是否相同，返回相同的位置，不区分大小写 */
ngx_int_t ngx_memn2cmp(u_char *s1, u_char *s2, size_t n1, size_t n2);           /* 比较两个指定长度的内存是否相同，也比较长的内存是否包含短的内存 */
ngx_int_t ngx_dns_strcmp(u_char *s1, u_char *s2);                               /* dns字符串比较 */

ngx_int_t ngx_atoi(u_char *line, size_t n);                                     /* 指定长度的字符串转换成数字 */
ngx_int_t ngx_atofp(u_char *line, size_t n, size_t point);                      /* dns字符串比较 */
ssize_t ngx_atosz(u_char *line, size_t n);                                      /* 指定长度的字符串转换成ssize_t类型数字 */
off_t ngx_atoof(u_char *line, size_t n);                                        /* 指定长度的字符串转换成off_t类型数字 */
time_t ngx_atotm(u_char *line, size_t n);                                       /* 指定长度的字符串转换成time_t类型数字 */
ngx_int_t ngx_hextoi(u_char *line, size_t n);                                   /* 指定长度的字符串转换成十六进制数字 */

u_char *ngx_hex_dump(u_char *dst, u_char *src, size_t len);                     /* 把数字转换成16进制的字符串 */


#define ngx_base64_encoded_length(len)  (((len + 2) / 3) * 4)                   /* base64编码长度 */
#define ngx_base64_decoded_length(len)  (((len + 3) / 4) * 3)                   /* base64解码长度 */

void ngx_encode_base64(ngx_str_t *dst, ngx_str_t *src);                         /* base64编码 */
ngx_int_t ngx_decode_base64(ngx_str_t *dst, ngx_str_t *src);                    /* base64解码 */
ngx_int_t ngx_decode_base64url(ngx_str_t *dst, ngx_str_t *src);                 /* base64url解码 */

uint32_t ngx_utf8_decode(u_char **p, size_t n);                                 /* utf8编码 */
size_t ngx_utf8_length(u_char *p, size_t n);                                    /* utf编码后长度 */
u_char *ngx_utf8_cpystrn(u_char *dst, u_char *src, size_t n, size_t len);       /* 复制指定长度的utf8字符串 */

/* 函数ngx_escape_uri的type参数 */
#define NGX_ESCAPE_URI            0     /* 对uri进行编码 */
#define NGX_ESCAPE_ARGS           1     /* 对参数进行编码 */
#define NGX_ESCAPE_URI_COMPONENT  2     /* 对uri组成部分进行个别编码，而不用于对整个uri进行编码 */
#define NGX_ESCAPE_HTML           3     /* 对html进行编码 */
#define NGX_ESCAPE_REFRESH        4     /* 对刷新信息进行编码 */
#define NGX_ESCAPE_MEMCACHED      5     /* 对缓存进行编码 */
#define NGX_ESCAPE_MAIL_AUTH      6     /* 对mail认证进行编码 */

/* 函数ngx_unescape_uri的type参数 */
#define NGX_UNESCAPE_URI       1        /* 对uri进行解码 */
#define NGX_UNESCAPE_REDIRECT  2        /* 对需要重定向uri进行解码 */

uintptr_t ngx_escape_uri(u_char *dst, u_char *src, size_t size,                 /* 对uri进行编码 */
    ngx_uint_t type);
void ngx_unescape_uri(u_char **dst, u_char **src, size_t size, ngx_uint_t type);/* 对uri的进行解码 */
uintptr_t ngx_escape_html(u_char *dst, u_char *src, size_t size);               /* 对html进行编码 */


typedef struct {
    ngx_rbtree_node_t         node;
    ngx_str_t                 str;
} ngx_str_node_t;


void ngx_str_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
ngx_str_node_t *ngx_str_rbtree_lookup(ngx_rbtree_t *rbtree, ngx_str_t *name,
    uint32_t hash);


void ngx_sort(void *base, size_t n, size_t size,                                /* 排序，主要是用于数组排序 */
    ngx_int_t (*cmp)(const void *, const void *));
#define ngx_qsort             qsort                                             /* 快速排序 */


#define ngx_value_helper(n)   #n
#define ngx_value(n)          ngx_value_helper(n)                               /* 把宏数字转换成字符串 */


#endif /* _NGX_STRING_H_INCLUDED_ */
