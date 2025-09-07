# RTOS核心功能设计文档

## 概述

本设计文档基于现有的STM32嵌入式系统架构，在保持与现有设备驱动框架、构建系统兼容的前提下，实现一个完整的RTOS内核。设计采用分层架构，充分利用现有的初始化系统和硬件抽象层。

## 架构设计

### 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (Application)                      │
├─────────────────────────────────────────────────────────────┤
│                RTOS API层 (RTOS API Layer)                  │
├─────────────────────────────────────────────────────────────┤
│  任务管理  │  调度器  │  同步原语  │  消息队列  │  定时器    │
├─────────────────────────────────────────────────────────────┤
│              内核服务层 (Kernel Services)                    │
├─────────────────────────────────────────────────────────────┤
│            硬件抽象层 (现有的驱动框架)                        │
├─────────────────────────────────────────────────────────────┤
│              ARM Cortex-M 硬件层                            │
└─────────────────────────────────────────────────────────────┘
```

### 与现有架构的集成

#### 1. 利用现有的初始化系统
- 扩展现有的 `w_cstart()` 函数，在调用 `main()` 前初始化RTOS
- 利用现有的设备初始化框架，将RTOS组件作为系统服务初始化
- 保持现有的链接器脚本和启动代码不变

#### 2. 扩展现有的TCB结构
```c
// 扩展现有的 include/kernel.h 中的 tcb_t
typedef struct task_control_block {
    uint32_t *stack_ptr;           // 栈指针（保持第一个位置，兼容现有代码）
    
    // 任务属性
    void (*entry)(void *);         // 任务入口函数
    void *parameter;               // 任务参数
    char name[16];                 // 任务名称
    uint8_t priority;              // 优先级 (0-7, 7最高)
    uint8_t state;                 // 任务状态
    
    // 栈管理
    uint32_t *stack_base;          // 栈基地址
    uint32_t stack_size;           // 栈大小
    
    // 调度信息
    uint32_t time_slice;           // 时间片
    uint32_t time_remaining;       // 剩余时间片
    uint32_t wake_time;            // 唤醒时间
    
    // 链表节点
    struct task_control_block *next;
    struct task_control_block *prev;
    
    // 同步对象
    void *waiting_object;          // 等待的同步对象
    uint32_t wait_timeout;         // 等待超时时间
    int wait_result;               // 等待结果
    
    // 统计信息
    uint32_t run_time;             // 累计运行时间
    uint32_t switch_count;         // 切换次数
    
    // 栈溢出检测
    uint32_t stack_canary;         // 栈金丝雀值
} tcb_t;
```

#### 3. 利用现有的汇编框架
- 扩展 `arch/arm/swap_helper.S` 中的上下文切换代码
- 利用现有的 `w_arm_pendsv` 和 `w_arm_svc` 中断处理框架
- 保持与现有 `pxCurrentTCB` 全局变量的兼容性

## 组件设计

### 1. 任务管理器 (Task Manager)

#### 数据结构
```c
// 任务状态枚举
typedef enum {
    TASK_STATE_READY = 0,          // 就绪
    TASK_STATE_RUNNING,            // 运行
    TASK_STATE_BLOCKED,            // 阻塞
    TASK_STATE_SUSPENDED,          // 挂起
    TASK_STATE_DELETED             // 已删除
} task_state_t;

// 任务句柄
typedef tcb_t* task_handle_t;

// 任务函数类型
typedef void (*task_func_t)(void *parameter);
```

#### 核心API
```c
// 任务管理API
task_handle_t task_create(const char *name, task_func_t entry, 
                         void *parameter, uint32_t stack_size, uint8_t priority);
int task_delete(task_handle_t task);
int task_suspend(task_handle_t task);
int task_resume(task_handle_t task);
void task_yield(void);
void task_delay(uint32_t ticks);
void task_delay_ms(uint32_t milliseconds);

// 任务信息查询
int task_get_info(task_handle_t task, task_info_t *info);
task_handle_t task_get_current(void);
const char* task_get_name(task_handle_t task);
```

#### 实现要点
- 任务栈从堆中动态分配，支持栈溢出检测
- 任务创建时自动初始化上下文，设置初始PC和SP
- 支持任务自删除和外部删除
- 实现任务状态转换的原子操作

### 2. 调度器 (Scheduler)

#### 调度策略
- **抢占式优先级调度**: 高优先级任务立即抢占低优先级任务
- **时间片轮转**: 同优先级任务采用时间片轮转
- **空闲任务**: 系统空闲时运行特殊的空闲任务

#### 数据结构
```c
// 就绪队列 - 每个优先级一个链表
typedef struct {
    tcb_t *head;
    tcb_t *tail;
    uint32_t count;
} ready_queue_t;

// 调度器状态
typedef struct {
    tcb_t *current_task;           // 当前运行任务
    ready_queue_t ready_queues[8]; // 就绪队列数组
    uint8_t ready_bitmap;          // 优先级位图
    uint32_t tick_count;           // 系统滴答计数
    uint32_t context_switches;     // 上下文切换次数
    bool scheduler_running;        // 调度器是否启动
    uint32_t lock_count;           // 调度锁计数
} scheduler_t;
```

#### 核心算法
```c
// 快速查找最高优先级
static inline uint8_t find_highest_priority(void) {
    if (scheduler.ready_bitmap == 0) return INVALID_PRIORITY;
    return 7 - __builtin_clz(scheduler.ready_bitmap);
}

// 任务选择算法
tcb_t* scheduler_select_next_task(void) {
    uint8_t highest_priority = find_highest_priority();
    if (highest_priority == INVALID_PRIORITY) {
        return &idle_task_tcb;
    }
    
    ready_queue_t *queue = &scheduler.ready_queues[highest_priority];
    tcb_t *next_task = queue->head;
    
    // 时间片轮转处理
    if (queue->count > 1) {
        remove_from_ready_queue(next_task);
        add_to_ready_queue_tail(next_task);
    }
    
    return next_task;
}
```

#### 与现有代码集成
- 扩展现有的 `w_scheduler()` 函数实现
- 利用现有的 `pxCurrentTCB` 全局变量
- 在 `SysTick_Handler` 中添加时间片管理

### 3. 同步原语

#### 互斥锁 (Mutex)
```c
typedef struct {
    tcb_t *owner;                  // 锁的持有者
    uint8_t original_priority;     // 持有者原始优先级
    tcb_t *wait_list;              // 等待队列
    uint32_t lock_count;           // 递归锁计数
    bool priority_inherited;       // 是否发生优先级继承
} mutex_t;

// API
mutex_t* mutex_create(void);
int mutex_lock(mutex_t *mutex, uint32_t timeout);
int mutex_unlock(mutex_t *mutex);
void mutex_delete(mutex_t *mutex);
```

#### 信号量 (Semaphore)
```c
typedef struct {
    uint32_t count;                // 当前计数
    uint32_t max_count;            // 最大计数
    tcb_t *wait_list;              // 等待队列
} semaphore_t;

// API
semaphore_t* sem_create(uint32_t initial_count, uint32_t max_count);
int sem_wait(semaphore_t *sem, uint32_t timeout);
int sem_post(semaphore_t *sem);
void sem_delete(semaphore_t *sem);
```

#### 优先级继承机制
- 当高优先级任务等待低优先级任务持有的互斥锁时
- 临时提升锁持有者的优先级到等待者的优先级
- 锁释放后恢复原始优先级

### 4. 消息传递

#### 消息队列
```c
typedef struct {
    void *buffer;                  // 消息缓冲区
    uint32_t item_size;            // 消息大小
    uint32_t max_items;            // 最大消息数
    uint32_t head;                 // 队列头
    uint32_t tail;                 // 队列尾
    uint32_t count;                // 当前消息数
    tcb_t *send_wait_list;         // 发送等待队列
    tcb_t *recv_wait_list;         // 接收等待队列
} message_queue_t;

// API
message_queue_t* queue_create(uint32_t item_size, uint32_t max_items);
int queue_send(message_queue_t *queue, const void *item, uint32_t timeout);
int queue_receive(message_queue_t *queue, void *item, uint32_t timeout);
```

#### 事件组
```c
typedef struct {
    uint32_t event_bits;           // 事件位
    tcb_t *wait_list;              // 等待队列
} event_group_t;

// API
event_group_t* event_group_create(void);
uint32_t event_group_wait(event_group_t *eg, uint32_t bits, 
                         bool wait_all, uint32_t timeout);
int event_group_set(event_group_t *eg, uint32_t bits);
```

### 5. 软件定时器

#### 数据结构
```c
typedef struct software_timer {
    struct software_timer *next;   // 链表节点
    uint32_t expire_time;          // 到期时间
    uint32_t period;               // 周期（0表示一次性）
    void (*callback)(void *);      // 回调函数
    void *parameter;               // 回调参数
    bool active;                   // 是否激活
} timer_t;

// 定时器管理器
typedef struct {
    timer_t *active_list;          // 活动定时器链表
    tcb_t *timer_task;             // 定时器任务
    message_queue_t *timer_queue;  // 定时器消息队列
} timer_manager_t;
```

#### API设计
```c
timer_t* timer_create(void (*callback)(void*), void *parameter, 
                     uint32_t period, bool auto_reload);
int timer_start(timer_t *timer, uint32_t delay);
int timer_stop(timer_t *timer);
int timer_reset(timer_t *timer);
void timer_delete(timer_t *timer);
```

### 6. 内存管理

#### 内存池算法
采用固定大小内存池 + 可变大小堆的混合策略：

```c
// 内存池配置
#define MEMORY_POOL_SIZES {32, 64, 128, 256, 512, 1024}
#define MEMORY_POOL_COUNTS {16, 12, 8, 6, 4, 2}

typedef struct {
    void *pool_start;              // 池起始地址
    uint32_t block_size;           // 块大小
    uint32_t block_count;          // 块数量
    uint32_t free_count;           // 空闲块数
    uint8_t *free_bitmap;          // 空闲位图
} memory_pool_t;
```

#### API设计
```c
void* malloc(size_t size);
void free(void *ptr);
void* calloc(size_t num, size_t size);
void* realloc(void *ptr, size_t size);

// 内存统计
typedef struct {
    uint32_t total_size;           // 总内存大小
    uint32_t used_size;            // 已使用大小
    uint32_t free_size;            // 空闲大小
    uint32_t largest_free;         // 最大空闲块
    uint32_t allocation_count;     // 分配次数
    uint32_t free_count;           // 释放次数
} memory_stats_t;

void get_memory_stats(memory_stats_t *stats);
```

## 错误处理和调试

### 错误码定义
```c
typedef enum {
    OS_OK = 0,                     // 成功
    OS_ERROR = -1,                 // 一般错误
    OS_TIMEOUT = -2,               // 超时
    OS_INVALID_PARAM = -3,         // 无效参数
    OS_NO_MEMORY = -4,             // 内存不足
    OS_RESOURCE_BUSY = -5,         // 资源忙
    OS_STACK_OVERFLOW = -6,        // 栈溢出
    OS_DEADLOCK = -7               // 死锁
} os_error_t;
```

### 调试接口
```c
// 任务信息
typedef struct {
    char name[16];
    uint8_t priority;
    uint8_t state;
    uint32_t stack_size;
    uint32_t stack_used;
    uint32_t run_time;
    uint32_t switch_count;
} task_info_t;

// 系统统计
typedef struct {
    uint32_t total_tasks;          // 总任务数
    uint32_t ready_tasks;          // 就绪任务数
    uint32_t blocked_tasks;        // 阻塞任务数
    uint32_t context_switches;     // 上下文切换次数
    uint32_t tick_count;           // 系统滴答数
    uint32_t cpu_usage;            // CPU使用率(%)
} system_stats_t;

// 调试API
void task_list_all(void);
void get_system_stats(system_stats_t *stats);
bool check_stack_overflow(task_handle_t task);
```

## 性能优化策略

### 1. 快速路径优化
- 使用内联函数减少函数调用开销
- 位操作快速查找最高优先级
- 缓存友好的数据结构布局

### 2. 中断延迟最小化
- 中断服务程序只做必要的工作
- 延迟处理放在任务上下文中
- 使用PendSV进行上下文切换

### 3. 内存访问优化
- 数据结构按缓存行对齐
- 减少内存分配/释放频率
- 使用内存池避免碎片

## 测试策略

### 单元测试
每个模块都有对应的测试用例：
- 任务管理测试
- 调度器测试
- 同步原语测试
- 消息传递测试
- 定时器测试
- 内存管理测试

### 集成测试
- 多任务协作测试
- 优先级抢占测试
- 同步原语组合测试
- 压力测试
- 长时间稳定性测试

### 性能基准测试
- 任务切换时间测试
- 中断响应时间测试
- 内存分配性能测试
- API调用开销测试

## 实现计划

### Phase 1: 核心调度系统 (2-3周)
1. 扩展现有TCB结构
2. 实现任务创建/删除
3. 完善调度器算法
4. 扩展上下文切换代码
5. 实现系统时钟管理

### Phase 2: 同步原语 (1-2周)
1. 实现互斥锁
2. 实现信号量
3. 添加优先级继承
4. 实现超时机制

### Phase 3: 高级功能 (2-3周)
1. 实现消息队列
2. 实现事件组
3. 实现软件定时器
4. 完善内存管理

### Phase 4: 优化和完善 (1-2周)
1. 性能优化
2. 调试接口
3. 错误处理
4. 文档和测试

## 与现有系统的兼容性

### 保持兼容的部分
- 现有的设备驱动接口不变
- 现有的构建系统和配置
- 现有的启动流程和初始化
- 现有的中断向量表

### 需要扩展的部分
- `kernel/init.c` 中添加RTOS初始化
- `arch/arm/swap_helper.S` 中完善上下文切换
- `include/kernel.h` 中扩展TCB结构
- 添加新的RTOS API头文件

这样的设计确保了RTOS功能的完整性，同时最大程度地保持了与现有系统的兼容性。