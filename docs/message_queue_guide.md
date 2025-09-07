# 消息队列实现指南

## 概述

消息队列（Message Queue）是RTOS中用于任务间通信的重要机制。它允许任务之间安全地传递任意大小的消息，支持缓冲和阻塞/非阻塞操作。

## 核心特性

### 1. 任务间通信
- 支持任意大小的消息传递
- FIFO（先进先出）消息处理顺序
- 线程安全的操作

### 2. 缓冲机制
- 可配置的队列长度
- 循环缓冲区实现
- 高效的内存使用

### 3. 阻塞/非阻塞操作
- 支持超时等待
- 支持立即返回
- 支持无限等待

### 4. ISR支持
- 提供ISR安全的发送函数
- 支持中断到任务的通信

## API接口

### 创建和删除

```c
// 创建消息队列
queue_handle_t queue_create(const char *name, uint32_t queue_length, uint32_t item_size);

// 删除消息队列
rtos_error_t queue_delete(queue_handle_t queue);
```

### 发送和接收

```c
// 发送消息（可阻塞）
rtos_error_t queue_send(queue_handle_t queue, const void *item, uint32_t timeout);

// 接收消息（可阻塞）
rtos_error_t queue_receive(queue_handle_t queue, void *item, uint32_t timeout);

// 从ISR发送消息（非阻塞）
rtos_error_t queue_send_from_isr(queue_handle_t queue, const void *item);
```

### 状态查询

```c
// 获取当前消息数量
uint32_t queue_get_count(queue_handle_t queue);

// 获取剩余空间
uint32_t queue_get_free_space(queue_handle_t queue);
```

## 使用示例

### 1. 基本消息传递

```c
// 创建消息队列
queue_handle_t msg_queue = queue_create("MSG_Q", 10, sizeof(uint32_t));

// 发送者任务
void sender_task(void *param) {
    uint32_t message = 0;
    while (1) {
        message++;
        
        // 发送消息（最多等待1秒）
        if (queue_send(msg_queue, &message, rtos_ms_to_ticks(1000)) == RTOS_OK) {
            LOG_DBG("Message %lu sent", message);
        }
        
        task_delay_ms(500);
    }
}

// 接收者任务
void receiver_task(void *param) {
    uint32_t received_msg;
    while (1) {
        // 接收消息（无限等待）
        if (queue_receive(msg_queue, &received_msg, RTOS_WAIT_FOREVER) == RTOS_OK) {
            LOG_DBG("Message %lu received", received_msg);
        }
    }
}
```

### 2. 结构体消息传递

```c
// 定义消息结构
typedef struct {
    uint8_t type;
    uint16_t data;
    uint32_t timestamp;
} sensor_msg_t;

// 创建结构体消息队列
queue_handle_t sensor_queue = queue_create("SENSOR_Q", 5, sizeof(sensor_msg_t));

// 发送结构体消息
void sensor_task(void *param) {
    sensor_msg_t msg;
    while (1) {
        msg.type = SENSOR_TEMPERATURE;
        msg.data = read_temperature();
        msg.timestamp = rtos_get_tick_count();
        
        queue_send(sensor_queue, &msg, rtos_ms_to_ticks(100));
        task_delay_ms(1000);
    }
}

// 处理结构体消息
void process_task(void *param) {
    sensor_msg_t msg;
    while (1) {
        if (queue_receive(sensor_queue, &msg, RTOS_WAIT_FOREVER) == RTOS_OK) {
            process_sensor_data(&msg);
        }
    }
}
```

### 3. 中断到任务通信

```c
// 字符队列用于串口接收
queue_handle_t uart_rx_queue = queue_create("UART_RX", 64, sizeof(char));

// UART接收中断处理
void UART_IRQHandler(void) {
    if (UART_GetITStatus(UART1, UART_IT_RXNE)) {
        char ch = UART_ReceiveData(UART1);
        
        // 从中断发送到队列（非阻塞）
        queue_send_from_isr(uart_rx_queue, &ch);
        
        UART_ClearITPendingBit(UART1, UART_IT_RXNE);
    }
}

// 串口处理任务
void uart_process_task(void *param) {
    char ch;
    while (1) {
        if (queue_receive(uart_rx_queue, &ch, RTOS_WAIT_FOREVER) == RTOS_OK) {
            // 处理接收到的字符
            process_uart_char(ch);
        }
    }
}
```

## CLI测试命令

系统提供了CLI命令来测试消息队列功能：

```bash
# 显示队列状态
queue_test status

# 发送数字到测试队列
queue_test test send 123

# 从测试队列接收
queue_test test recv

# 发送字符到字符队列
queue_test char send A

# 从字符队列接收
queue_test char recv
```

## 实现细节

### 1. 内存管理
- 使用静态内存池避免动态分配
- 支持最多8个消息队列
- 总缓冲区大小为2KB

### 2. 等待队列管理
- 分别管理发送和接收等待队列
- FIFO顺序确保公平性
- 支持超时处理

### 3. 直接传输优化
- 当有等待任务时直接传输消息
- 避免不必要的缓冲操作
- 提高通信效率

### 4. 循环缓冲区
- 高效的头尾指针管理
- 支持任意大小的消息
- 内存使用最优化

## 性能特点

### 优势
- **高效通信** - 直接传输优化
- **内存安全** - 静态分配，无碎片
- **实时性好** - 低延迟消息传递
- **易于使用** - 简洁的API接口

### 限制
- **队列数量** - 最多8个队列
- **总缓冲区** - 2KB限制
- **消息大小** - 受缓冲区限制

## 最佳实践

### 1. 队列设计
- 合理设置队列长度，避免过大或过小
- 根据消息频率选择合适的缓冲区大小
- 使用有意义的队列名称便于调试

### 2. 错误处理
- 始终检查返回值
- 合理设置超时时间
- 处理队列满和空的情况

### 3. 性能优化
- 优先使用直接传输
- 避免频繁的大消息传递
- 在ISR中使用专用的ISR函数

### 4. 调试技巧
- 使用CLI命令进行交互测试
- 监控队列状态和计数
- 记录消息传递的日志

## 总结

消息队列是RTOS中强大的通信机制，本实现提供了：
- 完整的消息传递功能
- 高效的内存管理
- 可靠的超时机制
- 丰富的测试和调试功能

通过合理使用消息队列，可以实现高效、安全的任务间通信，是构建复杂嵌入式应用的重要基础。