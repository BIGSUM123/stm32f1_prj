#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "stdint.h"

typedef struct {
    uint32_t *stack_ptr;      // 栈指针（必须是第一个成员）
    void (*entry)(void *);    // 线程入口函数
} tcb_t;

#endif // __KERNEL_H__
