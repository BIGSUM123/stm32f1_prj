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

/* Mutex Control Block */
typedef struct mutex_control_block {
    char name[16];                 // 互斥锁名称
    tcb_t *owner;                  // 当前持有者
    uint32_t lock_count;           // 递归锁计数
    tcb_t *wait_list_head;         // 等待队列头
    tcb_t *wait_list_tail;         // 等待队列尾
    uint8_t original_priority;     // 持有者原始优先级（用于优先级继承）
    bool is_recursive;             // 是否支持递归锁
    uint32_t magic;                // 魔数，用于检测对象有效性
} mutex_t;

#define MUTEX_MAGIC                0x4D555458  // "MUTX"

/* Semaphore Control Block */
typedef struct semaphore_control_block {
    char name[16];                 // 信号量名称
    uint32_t count;                // 当前计数值
    uint32_t max_count;            // 最大计数值
    tcb_t *wait_list_head;         // 等待队列头
    tcb_t *wait_list_tail;         // 等待队列尾
    bool is_binary;                // 是否为二进制信号量
    uint32_t magic;                // 魔数，用于检测对象有效性
} semaphore_t;

#define SEMAPHORE_MAGIC            0x53454D58  // "SEMX"

/* Message Queue Control Block */
typedef struct message_queue_control_block {
    char name[16];                 // 队列名称
    uint8_t *buffer;               // 消息缓冲区
    uint32_t item_size;            // 单个消息大小
    uint32_t queue_length;         // 队列最大长度
    uint32_t head;                 // 队列头索引
    uint32_t tail;                 // 队列尾索引
    uint32_t count;                // 当前消息数量
    tcb_t *send_wait_list_head;    // 发送等待队列头
    tcb_t *send_wait_list_tail;    // 发送等待队列尾
    tcb_t *recv_wait_list_head;    // 接收等待队列头
    tcb_t *recv_wait_list_tail;    // 接收等待队列尾
    uint32_t magic;                // 魔数，用于检测对象有效性
} message_queue_t;

#define QUEUE_MAGIC                0x51554555  // "QUEU"

/* Global Variables */
extern tcb_t *pxCurrentTCB;        // 当前任务指针（保持兼容性）

/* Kernel Internal Functions */
void w_scheduler(void);            // 调度器函数

/* Ready Queue Management (for synchronization objects) */
void add_to_ready_queue(tcb_t *task);
void remove_from_ready_queue(tcb_t *task);

/* Synchronization Object Management */
bool mutex_remove_waiting_task(tcb_t *task);
bool semaphore_remove_waiting_task(tcb_t *task);
bool queue_remove_waiting_task(tcb_t *task);

#endif // __KERNEL_H__
