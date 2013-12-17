
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{

#if (NGX_HAVE_ATOMIC_OPS)

    ngx_uint_t  i, n;

    for ( ;; ) {
        /* 如果lock为0,则说明没有进程持有锁，因此设置lock为value(为当前进程id),然后返回 */
        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
            return;
        }
        /* 如果cpu个数大于1(也就是多核),则进入spin-wait loop阶段 */
        if (ngx_ncpu > 1) {

            for (n = 1; n < spin; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }
                /*　然后重新获取锁，如果获得则直接返回　*/
                if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }
        /* 这个函数调用的是sched_yield，它会强迫当前运行的进程放弃占有处理器　*/
        ngx_sched_yield();
    }

#else

#if (NGX_THREADS)

#error ngx_spinlock() or ngx_atomic_cmp_set() are not defined !

#endif

#endif

}
