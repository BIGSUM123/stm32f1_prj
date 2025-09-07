#ifndef __RTOS_H__
#define __RTOS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RTOS Configuration */
#define RTOS_MAX_PRIORITIES         8
#define RTOS_MAX_TASKS              32
#define RTOS_TICK_RATE_HZ           100  // 1000Hz = 1ms per tick, matches HAL
#define RTOS_TIME_SLICE_TICKS       10
#define RTOS_STACK_CANARY           0xDEADBEEF
#define RTOS_STACK_FILL_PATTERN     0xA5A5A5A5
#define RTOS_IDLE_STACK_SIZE        64         

/* Error Codes */
typedef enum {
    RTOS_OK = 0,                    // 成功
    RTOS_ERROR = -1,                // 一般错误
    RTOS_TIMEOUT = -2,              // 超时
    RTOS_INVALID_PARAM = -3,        // 无效参数
    RTOS_NO_MEMORY = -4,            // 内存不足
    RTOS_RESOURCE_BUSY = -5,        // 资源忙
    RTOS_STACK_OVERFLOW = -6,       // 栈溢出
    RTOS_DEADLOCK = -7              // 死锁
} rtos_error_t;

/* Task States */
typedef enum {
    TASK_STATE_READY = 0,           // 就绪
    TASK_STATE_RUNNING,             // 运行
    TASK_STATE_BLOCKED,             // 阻塞
    TASK_STATE_SUSPENDED,           // 挂起
    TASK_STATE_DELETED              // 已删除
} task_state_t;

/* Task Function Type */
typedef void (*task_func_t)(void *parameter);

/* Forward Declarations */
struct task_control_block;
typedef struct task_control_block* task_handle_t;

struct mutex_control_block;
typedef struct mutex_control_block* mutex_handle_t;

struct semaphore_control_block;
typedef struct semaphore_control_block* semaphore_handle_t;

/* Wait Options */
#define RTOS_WAIT_FOREVER           0xFFFFFFFF
#define RTOS_NO_WAIT                0

/* Priority Definitions */
#define RTOS_PRIORITY_IDLE          0
#define RTOS_PRIORITY_LOW           1
#define RTOS_PRIORITY_NORMAL        3
#define RTOS_PRIORITY_HIGH          5
#define RTOS_PRIORITY_CRITICAL      7

/* RTOS Core Functions */
rtos_error_t rtos_init(void);
rtos_error_t rtos_start(void);
bool rtos_is_running(void);
bool rtos_is_initialized(void);

/* Task Management Functions */
task_handle_t task_create(const char *name, task_func_t entry, 
                         void *parameter, uint32_t stack_size, uint8_t priority);
rtos_error_t task_delete(task_handle_t task);
rtos_error_t task_suspend(task_handle_t task);
rtos_error_t task_resume(task_handle_t task);
void task_yield(void);
void task_delay(uint32_t ticks);
void task_delay_ms(uint32_t milliseconds);

/* Task Information */
task_handle_t task_get_current(void);
const char* task_get_name(task_handle_t task);
uint8_t task_get_priority(task_handle_t task);
task_state_t task_get_state(task_handle_t task);

/* Scheduler Functions */
void scheduler_lock(void);
void scheduler_unlock(void);
bool scheduler_is_locked(void);

/* System Functions */
uint32_t rtos_get_tick_count(void);
uint32_t rtos_ms_to_ticks(uint32_t milliseconds);
uint32_t rtos_ticks_to_ms(uint32_t ticks);

/* Mutex Functions */
mutex_handle_t mutex_create(const char *name);
rtos_error_t mutex_delete(mutex_handle_t mutex);
rtos_error_t mutex_lock(mutex_handle_t mutex, uint32_t timeout);
rtos_error_t mutex_unlock(mutex_handle_t mutex);
task_handle_t mutex_get_owner(mutex_handle_t mutex);

/* Semaphore Functions */
semaphore_handle_t sem_create_binary(const char *name, uint32_t initial_count);
semaphore_handle_t sem_create_counting(const char *name, uint32_t max_count, uint32_t initial_count);
rtos_error_t sem_delete(semaphore_handle_t sem);
rtos_error_t sem_wait(semaphore_handle_t sem, uint32_t timeout);
rtos_error_t sem_post(semaphore_handle_t sem);
uint32_t sem_get_count(semaphore_handle_t sem);

#ifdef __cplusplus
}
#endif

#endif /* __RTOS_H__ */