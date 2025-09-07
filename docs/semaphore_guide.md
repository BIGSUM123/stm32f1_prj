# 信号量实现指南

## 概述

本文档描述了RTOS中信号量（Semaphore）的实现和使用方法。信号量是一种用于任务间同步和资源管理的重要机制。

## 信号量类型

### 1. 二进制信号量（Binary Semaphore）
- 计数值只能是0或1
- 主要用于任务间的同步
- 类似于互斥锁，但没有所有权概念

### 2. 计数信号量（Counting Semaphore）
- 计数值可以是0到最大值之间的任何值
- 主要用于资源池管理
- 可以控制同时访问资源的任务数量

## API接口

### 创建信号量

```c
// 创建二进制信号量
semaphore_handle_t sem_create_binary(const char *name, uint32_t initial_count);

// 创建计数信号量
semaphore_handle_t sem_create_counting(const char *name, uint32_t max_count, uint32_t initial_count);
```

### 操作信号量

```c
// 等待信号量（P操作，获取）
rtos_error_t sem_wait(semaphore_handle_t sem, uint32_t timeout);

// 释放信号量（V操作，释放）
rtos_error_t sem_post(semaphore_handle_t sem);

// 获取当前计数值
uint32_t sem_get_count(semaphore_handle_t sem);

// 删除信号量
rtos_error_t sem_delete(semaphore_handle_t sem);
```

## 使用示例

### 1. 任务同步示例

```c
// 创建二进制信号量用于同步
semaphore_handle_t sync_sem = sem_create_binary("SYNC", 0);

// 生产者任务
void producer_task(void *param) {
    while (1) {
        // 生产数据
        produce_data();
        
        // 通知消费者
        sem_post(sync_sem);
        
        task_delay_ms(1000);
    }
}

// 消费者任务
void consumer_task(void *param) {
    while (1) {
        // 等待生产者通知
        if (sem_wait(sync_sem, RTOS_WAIT_FOREVER) == RTOS_OK) {
            // 消费数据
            consume_data();
        }
    }
}
```

### 2. 资源池管理示例

```c
// 创建计数信号量管理资源池（最多5个资源）
semaphore_handle_t resource_sem = sem_create_counting("RESOURCE", 5, 5);

// 任务获取资源
void worker_task(void *param) {
    while (1) {
        // 获取资源（最多等待2秒）
        if (sem_wait(resource_sem, rtos_ms_to_ticks(2000)) == RTOS_OK) {
            // 使用资源
            use_resource();
            
            // 释放资源
            sem_post(resource_sem);
        } else {
            LOG_DBG("Failed to acquire resource");
        }
        
        task_delay_ms(500);
    }
}
```

## CLI测试命令

系统提供了CLI命令来测试信号量功能：

```bash
# 显示信号量状态
sem_test status

# 发布二进制信号量
sem_test binary post

# 等待二进制信号量
sem_test binary wait

# 发布计数信号量
sem_test counting post

# 等待计数信号量
sem_test counting wait
```

## 实现特性

### 1. 等待队列管理
- 使用FIFO（先进先出）策略管理等待任务
- 确保公平性，避免任务饥饿

### 2. 超时支持
- 支持无限等待（RTOS_WAIT_FOREVER）
- 支持立即返回（RTOS_NO_WAIT）
- 支持指定超时时间

### 3. 错误处理
- 参数验证
- 溢出检测（计数信号量）
- 超时处理

### 4. 内存管理
- 使用静态内存池，避免动态分配
- 支持最多16个信号量实例

## 注意事项

### 1. 优先级反转
- 信号量不支持优先级继承
- 如需避免优先级反转，请使用互斥锁

### 2. 死锁预防
- 避免循环等待
- 使用超时机制
- 合理设计任务优先级

### 3. 性能考虑
- 信号量操作在中断禁用状态下执行
- 尽量减少临界区时间
- 避免在中断服务程序中长时间等待

## 测试验证

系统包含了完整的信号量测试任务：

1. **生产者任务**：定期发布信号量
2. **消费者任务**：等待并获取信号量
3. **CLI测试**：交互式测试信号量操作

通过这些测试可以验证：
- 信号量的基本功能
- 超时机制
- 任务同步
- 资源管理

## 总结

信号量是RTOS中重要的同步原语，本实现提供了：
- 完整的二进制和计数信号量支持
- 可靠的超时机制
- 公平的等待队列管理
- 丰富的测试和调试功能

通过合理使用信号量，可以实现高效的任务间同步和资源管理。