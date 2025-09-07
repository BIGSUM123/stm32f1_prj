#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <stdbool.h>
#include "rtos.h"

/* Extended Task Control Block */
typedef struct task_control_block {
    /* Context Information - MUST be first for compatibility */
    uint32_t *stack_ptr;           // 当前栈指针（必须是第一个成员）
    
    /* Task Attributes */
    task_func_t entry;             // 任务入口函数
    void *parameter;               // 任务参数
    char name[16];                 // 任务名称
    uint8_t priority;              // 优先级 (0-7, 7最高)
    uint8_t state;                 // 任务状态
    
    /* Stack Management */
    uint32_t *stack_base;          // 栈基地址
    uint32_t stack_size;           // 栈大小
    
    /* Scheduling Information */
    uint32_t time_slice;           // 时间片
    uint32_t time_remaining;       // 剩余时间片
    uint32_t wake_time;            // 唤醒时间
    
    /* List Nodes */
    struct task_control_block *next;
    struct task_control_block *prev;
    
    /* Synchronization Objects */
    void *waiting_object;          // 等待的同步对象
    uint32_t wait_timeout;         // 等待超时时间
    int wait_result;               // 等待结果
    
    /* Statistics */
    uint32_t run_time;             // 累计运行时间
    uint32_t switch_count;         // 切换次数
    
    /* Stack Overflow Detection */
    uint32_t stack_canary;         // 栈金丝雀值
} tcb_t;

/* Global Variables */
extern tcb_t *pxCurrentTCB;        // 当前任务指针（保持兼容性）

/* Kernel Internal Functions */
void w_scheduler(void);            // 调度器函数

#endif // __KERNEL_H__
