/**
 * @file message_queue.c
 * @brief Message Queue implementation for RTOS
 * @author RTOS Team
 * @date 2024
 */

#include "kernel.h"
#include "rtos.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>

/* ARM CMSIS includes for intrinsic functions */
#if defined(__ARM_ARCH) || defined(STM32F407xx) || defined(STM32F103xx)
#include "stm32f407xx.h" // STM32F407 specific definitions (includes core_cm4.h)
#else
/* Fallback definitions for non-ARM platforms */
#define __disable_irq()
#define __enable_irq()
#define SCB_ICSR_PENDSVSET_Msk 0
typedef struct {
	uint32_t ICSR;
} SCB_Type;
extern SCB_Type *SCB;
#endif

/* Configuration */
#define RTOS_MAX_QUEUES        8
#define RTOS_QUEUE_BUFFER_SIZE 2048 // Total buffer size for all queues

/* Static queue pool */
static message_queue_t queue_pool[RTOS_MAX_QUEUES];
static bool queue_pool_used[RTOS_MAX_QUEUES];
static uint8_t queue_buffer_pool[RTOS_QUEUE_BUFFER_SIZE];
static uint32_t queue_buffer_offset = 0;

/* Internal function declarations */
static void add_to_queue_send_wait_list(message_queue_t *queue, tcb_t *task);
static tcb_t *remove_from_queue_send_wait_list(message_queue_t *queue);
static void add_to_queue_recv_wait_list(message_queue_t *queue, tcb_t *task);
static tcb_t *remove_from_queue_recv_wait_list(message_queue_t *queue);
static bool remove_task_from_queue_wait_lists(message_queue_t *queue, tcb_t *task);
static bool queue_is_full(message_queue_t *queue);
static bool queue_is_empty(message_queue_t *queue);
static void queue_put_item(message_queue_t *queue, const void *item);
static void queue_get_item(message_queue_t *queue, void *item);

/**
 * @brief Create a message queue
 * @param name Queue name (optional, can be NULL)
 * @param queue_length Maximum number of messages in queue
 * @param item_size Size of each message in bytes
 * @return Queue handle or NULL if failed
 */
queue_handle_t queue_create(const char *name, uint32_t queue_length, uint32_t item_size)
{
	if (queue_length == 0 || item_size == 0) {
		return NULL;
	}

	// Calculate required buffer size
	uint32_t buffer_size = queue_length * item_size;

	__disable_irq();

	// Check if we have enough buffer space
	if (queue_buffer_offset + buffer_size > RTOS_QUEUE_BUFFER_SIZE) {
		__enable_irq();
		LOG_DBG("Queue create failed: insufficient buffer space");
		return NULL;
	}

	// Find free queue slot
	message_queue_t *queue = NULL;
	for (int i = 0; i < RTOS_MAX_QUEUES; i++) {
		if (!queue_pool_used[i]) {
			queue = &queue_pool[i];
			queue_pool_used[i] = true;
			break;
		}
	}

	if (queue == NULL) {
		__enable_irq();
		LOG_DBG("Queue create failed: no free slots");
		return NULL;
	}

	// Initialize queue
	memset(queue, 0, sizeof(message_queue_t));
	if (name != NULL) {
		strncpy(queue->name, name, sizeof(queue->name) - 1);
		queue->name[sizeof(queue->name) - 1] = '\0';
	} else {
		strcpy(queue->name, "UNNAMED");
	}

	// Allocate buffer from pool
	queue->buffer = &queue_buffer_pool[queue_buffer_offset];
	queue_buffer_offset += buffer_size;

	queue->item_size = item_size;
	queue->queue_length = queue_length;
	queue->head = 0;
	queue->tail = 0;
	queue->count = 0;
	queue->send_wait_list_head = NULL;
	queue->send_wait_list_tail = NULL;
	queue->recv_wait_list_head = NULL;
	queue->recv_wait_list_tail = NULL;
	queue->magic = QUEUE_MAGIC;

	__enable_irq();

	LOG_DBG("Queue created: %s (length=%lu, item_size=%lu)", queue->name, queue_length,
		item_size);

	return queue;
}

/**
 * @brief Delete a message queue
 * @param queue Queue handle
 * @return RTOS_OK if successful
 */
rtos_error_t queue_delete(queue_handle_t queue)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC) {
		return RTOS_INVALID_PARAM;
	}

	__disable_irq();

	// Wake up all waiting tasks with error
	while (queue->send_wait_list_head != NULL) {
		tcb_t *task = remove_from_queue_send_wait_list(queue);
		if (task != NULL) {
			task->wait_result = RTOS_ERROR;
			task->waiting_object = NULL;
			add_to_ready_queue(task);
		}
	}

	while (queue->recv_wait_list_head != NULL) {
		tcb_t *task = remove_from_queue_recv_wait_list(queue);
		if (task != NULL) {
			task->wait_result = RTOS_ERROR;
			task->waiting_object = NULL;
			add_to_ready_queue(task);
		}
	}

	// Mark queue as free
	queue->magic = 0;
	for (int i = 0; i < RTOS_MAX_QUEUES; i++) {
		if (&queue_pool[i] == queue) {
			queue_pool_used[i] = false;
			break;
		}
	}

	__enable_irq();

	LOG_DBG("Queue deleted: %s", queue->name);
	return RTOS_OK;
}

/**
 * @brief Send a message to queue
 * @param queue Queue handle
 * @param item Pointer to message data
 * @param timeout Timeout in ticks (RTOS_WAIT_FOREVER for infinite wait)
 * @return RTOS_OK if successful
 */
rtos_error_t queue_send(queue_handle_t queue, const void *item, uint32_t timeout)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC || item == NULL) {
		return RTOS_INVALID_PARAM;
	}

	if (!rtos_is_running() || pxCurrentTCB == NULL) {
		return RTOS_ERROR;
	}

	__disable_irq();

	// Check if there are waiting receivers
	tcb_t *waiting_receiver = remove_from_queue_recv_wait_list(queue);
	if (waiting_receiver != NULL) {
		// Direct transfer to waiting receiver
		memcpy(waiting_receiver->waiting_object, item, queue->item_size);
		waiting_receiver->waiting_object = NULL;
		waiting_receiver->wait_result = RTOS_OK;
		add_to_ready_queue(waiting_receiver);

		__enable_irq();

		LOG_DBG("Queue message transferred directly to %s: %s", waiting_receiver->name,
			queue->name);

		// Trigger context switch if needed
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
		return RTOS_OK;
	}

	// Check if queue has space
	if (!queue_is_full(queue)) {
		// Queue has space, add message
		queue_put_item(queue, item);
		__enable_irq();

		LOG_DBG("Message sent to queue: %s (count=%lu)", queue->name, queue->count);
		return RTOS_OK;
	}

	// Queue is full
	if (timeout == RTOS_NO_WAIT) {
		__enable_irq();
		return RTOS_TIMEOUT;
	}

	// Set up waiting information
	pxCurrentTCB->waiting_object = (void *)item; // Store item pointer temporarily
	pxCurrentTCB->wait_timeout =
		(timeout == RTOS_WAIT_FOREVER) ? 0 : (rtos_get_tick_count() + timeout);
	pxCurrentTCB->wait_result = RTOS_OK;

	// Remove from ready queue and add to send wait list
	remove_from_ready_queue(pxCurrentTCB);
	pxCurrentTCB->state = TASK_STATE_BLOCKED;
	add_to_queue_send_wait_list(queue, pxCurrentTCB);

	__enable_irq();

	LOG_DBG("Task %s waiting to send to queue: %s", pxCurrentTCB->name, queue->name);

	// Trigger context switch
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	// When we return here, we either sent the message or timed out
	return pxCurrentTCB->wait_result;
}

/**
 * @brief Receive a message from queue
 * @param queue Queue handle
 * @param item Pointer to buffer for received message
 * @param timeout Timeout in ticks (RTOS_WAIT_FOREVER for infinite wait)
 * @return RTOS_OK if successful
 */
rtos_error_t queue_receive(queue_handle_t queue, void *item, uint32_t timeout)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC || item == NULL) {
		return RTOS_INVALID_PARAM;
	}

	if (!rtos_is_running() || pxCurrentTCB == NULL) {
		return RTOS_ERROR;
	}

	__disable_irq();

	// Check if queue has messages
	if (!queue_is_empty(queue)) {
		// Queue has messages, get one
		queue_get_item(queue, item);

		// Check if there are waiting senders
		tcb_t *waiting_sender = remove_from_queue_send_wait_list(queue);
		if (waiting_sender != NULL) {
			// Add sender's message to queue
			queue_put_item(queue, (const void *)waiting_sender->waiting_object);

			// Clear waiting state before changing task state
			waiting_sender->waiting_object = NULL;
			waiting_sender->wait_result = RTOS_OK;
			add_to_ready_queue(waiting_sender);

			LOG_DBG("Queue message received and sender unblocked: %s", queue->name);
		}

		__enable_irq();

		LOG_DBG("Message received from queue: %s (count=%lu)", queue->name, queue->count);

		// Trigger context switch if we unblocked a sender
		if (waiting_sender != NULL) {
			SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
		}

		return RTOS_OK;
	}

	// Queue is empty
	if (timeout == RTOS_NO_WAIT) {
		__enable_irq();
		return RTOS_TIMEOUT;
	}

	// Set up waiting information
	pxCurrentTCB->waiting_object = item; // Store receive buffer pointer
	pxCurrentTCB->wait_timeout =
		(timeout == RTOS_WAIT_FOREVER) ? 0 : (rtos_get_tick_count() + timeout);
	pxCurrentTCB->wait_result = RTOS_OK;

	// Remove from ready queue and add to receive wait list
	remove_from_ready_queue(pxCurrentTCB);
	pxCurrentTCB->state = TASK_STATE_BLOCKED;
	add_to_queue_recv_wait_list(queue, pxCurrentTCB);

	__enable_irq();

	LOG_DBG("Task %s waiting to receive from queue: %s", pxCurrentTCB->name, queue->name);

	// Trigger context switch
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	// When we return here, we either received the message or timed out
	return pxCurrentTCB->wait_result;
}

/**
 * @brief Send a message to queue from ISR (non-blocking)
 * @param queue Queue handle
 * @param item Pointer to message data
 * @return RTOS_OK if successful
 */
rtos_error_t queue_send_from_isr(queue_handle_t queue, const void *item)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC || item == NULL) {
		return RTOS_INVALID_PARAM;
	}

	// Check if there are waiting receivers
	tcb_t *waiting_receiver = remove_from_queue_recv_wait_list(queue);
	if (waiting_receiver != NULL) {
		// Direct transfer to waiting receiver
		memcpy(waiting_receiver->waiting_object, item, queue->item_size);

		// Clear waiting state before changing task state
		waiting_receiver->waiting_object = NULL;
		waiting_receiver->wait_result = RTOS_OK;
		add_to_ready_queue(waiting_receiver);

		// Request context switch
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
		return RTOS_OK;
	}

	// Check if queue has space
	if (!queue_is_full(queue)) {
		// Queue has space, add message
		queue_put_item(queue, item);
		return RTOS_OK;
	}

	// Queue is full, cannot block in ISR
	return RTOS_RESOURCE_BUSY;
}

/**
 * @brief Get current number of messages in queue
 * @param queue Queue handle
 * @return Current message count or 0 if invalid
 */
uint32_t queue_get_count(queue_handle_t queue)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC) {
		return 0;
	}

	return queue->count;
}

/**
 * @brief Get number of free spaces in queue
 * @param queue Queue handle
 * @return Free space count or 0 if invalid
 */
uint32_t queue_get_free_space(queue_handle_t queue)
{
	if (queue == NULL || queue->magic != QUEUE_MAGIC) {
		return 0;
	}

	return queue->queue_length - queue->count;
}

/* Internal helper functions */

/**
 * @brief Check if queue is full
 */
static bool queue_is_full(message_queue_t *queue)
{
	return queue->count >= queue->queue_length;
}

/**
 * @brief Check if queue is empty
 */
static bool queue_is_empty(message_queue_t *queue)
{
	return queue->count == 0;
}

/**
 * @brief Put item into queue buffer
 */
static void queue_put_item(message_queue_t *queue, const void *item)
{
	uint8_t *dest = queue->buffer + (queue->tail * queue->item_size);
	memcpy(dest, item, queue->item_size);

	queue->tail = (queue->tail + 1) % queue->queue_length;
	queue->count++;
}

/**
 * @brief Get item from queue buffer
 */
static void queue_get_item(message_queue_t *queue, void *item)
{
	uint8_t *src = queue->buffer + (queue->head * queue->item_size);
	memcpy(item, src, queue->item_size);

	queue->head = (queue->head + 1) % queue->queue_length;
	queue->count--;
}

/**
 * @brief Add task to queue send wait list (FIFO order)
 */
static void add_to_queue_send_wait_list(message_queue_t *queue, tcb_t *task)
{
	task->next = NULL;
	task->prev = NULL;

	if (queue->send_wait_list_head == NULL) {
		queue->send_wait_list_head = task;
		queue->send_wait_list_tail = task;
	} else {
		queue->send_wait_list_tail->next = task;
		task->prev = queue->send_wait_list_tail;
		queue->send_wait_list_tail = task;
	}
}

/**
 * @brief Remove first task from queue send wait list
 */
static tcb_t *remove_from_queue_send_wait_list(message_queue_t *queue)
{
	if (queue->send_wait_list_head == NULL) {
		return NULL;
	}

	tcb_t *task = queue->send_wait_list_head;

	if (task->next != NULL) {
		task->next->prev = NULL;
		queue->send_wait_list_head = task->next;
	} else {
		queue->send_wait_list_head = NULL;
		queue->send_wait_list_tail = NULL;
	}

	task->next = NULL;
	task->prev = NULL;

	return task;
}

/**
 * @brief Add task to queue receive wait list (FIFO order)
 */
static void add_to_queue_recv_wait_list(message_queue_t *queue, tcb_t *task)
{
	task->next = NULL;
	task->prev = NULL;

	if (queue->recv_wait_list_head == NULL) {
		queue->recv_wait_list_head = task;
		queue->recv_wait_list_tail = task;
	} else {
		queue->recv_wait_list_tail->next = task;
		task->prev = queue->recv_wait_list_tail;
		queue->recv_wait_list_tail = task;
	}
}

/**
 * @brief Remove first task from queue receive wait list
 */
static tcb_t *remove_from_queue_recv_wait_list(message_queue_t *queue)
{
	if (queue->recv_wait_list_head == NULL) {
		return NULL;
	}

	tcb_t *task = queue->recv_wait_list_head;

	if (task->next != NULL) {
		task->next->prev = NULL;
		queue->recv_wait_list_head = task->next;
	} else {
		queue->recv_wait_list_head = NULL;
		queue->recv_wait_list_tail = NULL;
	}

	task->next = NULL;
	task->prev = NULL;

	return task;
}

/**
 * @brief Remove specific task from queue wait lists
 */
static bool remove_task_from_queue_wait_lists(message_queue_t *queue, tcb_t *task)
{
	if (queue == NULL || task == NULL) {
		return false;
	}

	// Try send wait list first
	tcb_t *current = queue->send_wait_list_head;
	while (current != NULL) {
		if (current == task) {
			// Found in send wait list, remove it
			if (task->prev != NULL) {
				task->prev->next = task->next;
			} else {
				queue->send_wait_list_head = task->next;
			}

			if (task->next != NULL) {
				task->next->prev = task->prev;
			} else {
				queue->send_wait_list_tail = task->prev;
			}

			task->next = NULL;
			task->prev = NULL;
			return true;
		}
		current = current->next;
	}

	// Try receive wait list
	current = queue->recv_wait_list_head;
	while (current != NULL) {
		if (current == task) {
			// Found in receive wait list, remove it
			if (task->prev != NULL) {
				task->prev->next = task->next;
			} else {
				queue->recv_wait_list_head = task->next;
			}

			if (task->next != NULL) {
				task->next->prev = task->prev;
			} else {
				queue->recv_wait_list_tail = task->prev;
			}

			task->next = NULL;
			task->prev = NULL;
			return true;
		}
		current = current->next;
	}

	return false; // Task not found in any wait list
}

/**
 * @brief Remove task from any queue wait list (called by timeout handler)
 * @param task Task to remove from wait lists
 * @return true if task was found and removed
 */
bool queue_remove_waiting_task(tcb_t *task)
{
	if (task == NULL || task->waiting_object == NULL) {
		return false;
	}

	// We need to find which queue this task is waiting on
	// Since we don't have a direct reference, we'll search all queues
	for (int i = 0; i < RTOS_MAX_QUEUES; i++) {
		if (queue_pool_used[i]) {
			message_queue_t *queue = &queue_pool[i];
			if (queue->magic == QUEUE_MAGIC) {
				if (remove_task_from_queue_wait_lists(queue, task)) {
					return true;
				}
			}
		}
	}

	return false;
}