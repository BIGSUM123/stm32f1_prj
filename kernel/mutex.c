/**
 * @file mutex.c
 * @brief Mutex (Mutual Exclusion) implementation for RTOS
 * @author RTOS Team
 * @date 2024
 */

#include "kernel.h"
#include "rtos.h"
#include "log.h"
#include <string.h>

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
#define RTOS_MAX_MUTEXES 16

/* Static mutex pool */
static mutex_t mutex_pool[RTOS_MAX_MUTEXES];
static bool mutex_pool_used[RTOS_MAX_MUTEXES];

/* Internal function declarations */
static void add_to_mutex_wait_list(mutex_t *mutex, tcb_t *task);
static tcb_t *remove_from_mutex_wait_list(mutex_t *mutex);
static bool remove_task_from_mutex_wait_list(mutex_t *mutex, tcb_t *task);
static void priority_inheritance(mutex_t *mutex, tcb_t *task);
static void priority_restore(mutex_t *mutex);

/**
 * @brief Create a mutex
 * @param name Mutex name (optional, can be NULL)
 * @return Mutex handle or NULL if failed
 */
mutex_handle_t mutex_create(const char *name)
{
	__disable_irq();

	// Find free mutex slot
	mutex_t *mutex = NULL;
	for (int i = 0; i < RTOS_MAX_MUTEXES; i++) {
		if (!mutex_pool_used[i]) {
			mutex = &mutex_pool[i];
			mutex_pool_used[i] = true;
			break;
		}
	}

	if (mutex == NULL) {
		__enable_irq();
		LOG_DBG("Mutex create failed: no free slots");
		return NULL;
	}

	// Initialize mutex
	memset(mutex, 0, sizeof(mutex_t));
	if (name != NULL) {
		strncpy(mutex->name, name, sizeof(mutex->name) - 1);
		mutex->name[sizeof(mutex->name) - 1] = '\0';
	} else {
		strcpy(mutex->name, "UNNAMED");
	}

	mutex->owner = NULL;
	mutex->lock_count = 0;
	mutex->wait_list_head = NULL;
	mutex->wait_list_tail = NULL;
	mutex->original_priority = 0;
	mutex->is_recursive = true; // Support recursive locking by default
	mutex->magic = MUTEX_MAGIC;

	__enable_irq();

	LOG_DBG("Mutex created: %s", mutex->name);
	return mutex;
}

/**
 * @brief Delete a mutex
 * @param mutex Mutex handle
 * @return RTOS_OK if successful
 */
rtos_error_t mutex_delete(mutex_handle_t mutex)
{
	if (mutex == NULL || mutex->magic != MUTEX_MAGIC) {
		return RTOS_INVALID_PARAM;
	}

	__disable_irq();

	// Check if mutex is locked
	if (mutex->owner != NULL) {
		__enable_irq();
		LOG_DBG("Cannot delete locked mutex: %s", mutex->name);
		return RTOS_RESOURCE_BUSY;
	}

	// Wake up all waiting tasks with error
	while (mutex->wait_list_head != NULL) {
		tcb_t *task = remove_from_mutex_wait_list(mutex);
		if (task != NULL) {
			task->wait_result = RTOS_ERROR;
			task->waiting_object = NULL;
			// Add back to ready queue
			add_to_ready_queue(task);
		}
	}

	// Mark mutex as free
	mutex->magic = 0;
	for (int i = 0; i < RTOS_MAX_MUTEXES; i++) {
		if (&mutex_pool[i] == mutex) {
			mutex_pool_used[i] = false;
			break;
		}
	}

	__enable_irq();

	LOG_DBG("Mutex deleted: %s", mutex->name);
	return RTOS_OK;
}

/**
 * @brief Lock a mutex
 * @param mutex Mutex handle
 * @param timeout Timeout in ticks (RTOS_WAIT_FOREVER for infinite wait)
 * @return RTOS_OK if successful
 */
rtos_error_t mutex_lock(mutex_handle_t mutex, uint32_t timeout)
{
	if (mutex == NULL || mutex->magic != MUTEX_MAGIC) {
		return RTOS_INVALID_PARAM;
	}

	if (!rtos_is_running() || pxCurrentTCB == NULL) {
		return RTOS_ERROR;
	}

	__disable_irq();

	// Check if mutex is available
	if (mutex->owner == NULL) {
		// Mutex is free, acquire it
		mutex->owner = pxCurrentTCB;
		mutex->lock_count = 1;
		mutex->original_priority = pxCurrentTCB->priority;

		__enable_irq();
		LOG_DBG("Mutex locked by %s: %s", pxCurrentTCB->name, mutex->name);
		return RTOS_OK;
	}

	// Check if current task already owns the mutex (recursive lock)
	if (mutex->owner == pxCurrentTCB) {
		if (mutex->is_recursive) {
			mutex->lock_count++;
			__enable_irq();
			LOG_DBG("Mutex recursive lock by %s: %s (count=%lu)", pxCurrentTCB->name,
				mutex->name, mutex->lock_count);
			return RTOS_OK;
		} else {
			__enable_irq();
			LOG_DBG("Non-recursive mutex already owned: %s", mutex->name);
			return RTOS_DEADLOCK;
		}
	}

	// Mutex is owned by another task
	if (timeout == RTOS_NO_WAIT) {
		__enable_irq();
		return RTOS_TIMEOUT;
	}

	// Set up waiting information
	pxCurrentTCB->waiting_object = mutex;
	pxCurrentTCB->wait_timeout =
		(timeout == RTOS_WAIT_FOREVER) ? 0 : (rtos_get_tick_count() + timeout);
	pxCurrentTCB->wait_result = RTOS_OK;

	// IMPORTANT: Remove from ready queue BEFORE adding to wait list
	// because add_to_mutex_wait_list will modify next/prev pointers
	remove_from_ready_queue(pxCurrentTCB);
	pxCurrentTCB->state = TASK_STATE_BLOCKED;

	// Now add to mutex wait list (safe to modify next/prev now)
	add_to_mutex_wait_list(mutex, pxCurrentTCB);

	// Implement priority inheritance
	priority_inheritance(mutex, pxCurrentTCB);

	__enable_irq();

	// Trigger context switch
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	// When we return here, we either got the mutex or timed out
	return pxCurrentTCB->wait_result;
}

/**
 * @brief Unlock a mutex
 * @param mutex Mutex handle
 * @return RTOS_OK if successful
 */
rtos_error_t mutex_unlock(mutex_handle_t mutex)
{
	if (mutex == NULL || mutex->magic != MUTEX_MAGIC) {
		return RTOS_INVALID_PARAM;
	}

	if (!rtos_is_running() || pxCurrentTCB == NULL) {
		return RTOS_ERROR;
	}

	__disable_irq();

	// Check if current task owns the mutex
	if (mutex->owner != pxCurrentTCB) {
		__enable_irq();
		LOG_DBG("Task %s doesn't own mutex %s", pxCurrentTCB->name, mutex->name);
		return RTOS_ERROR;
	}

	// Decrement lock count for recursive locks
	mutex->lock_count--;
	if (mutex->lock_count > 0) {
		__enable_irq();
		LOG_DBG("Mutex recursive unlock by %s: %s (count=%lu)", pxCurrentTCB->name,
			mutex->name, mutex->lock_count);
		return RTOS_OK;
	}

	// Restore original priority if it was elevated
	priority_restore(mutex);

	// Check if there are waiting tasks
	tcb_t *next_owner = remove_from_mutex_wait_list(mutex);
	if (next_owner != NULL) {
		// Transfer ownership to waiting task
		mutex->owner = next_owner;
		mutex->lock_count = 1;
		mutex->original_priority = next_owner->priority;

		// Wake up the new owner
		next_owner->waiting_object = NULL;
		next_owner->wait_result = RTOS_OK;

		// Add to ready queue
		add_to_ready_queue(next_owner);

		LOG_DBG("Mutex transferred from %s to %s: %s", pxCurrentTCB->name, next_owner->name,
			mutex->name);
	} else {
		// No waiting tasks, mutex becomes free
		mutex->owner = NULL;
		LOG_DBG("Mutex unlocked by %s: %s", pxCurrentTCB->name, mutex->name);
	}

	__enable_irq();

	// Trigger context switch if needed
	if (next_owner != NULL) {
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}

	return RTOS_OK;
}

/**
 * @brief Get mutex owner
 * @param mutex Mutex handle
 * @return Task handle of owner or NULL
 */
task_handle_t mutex_get_owner(mutex_handle_t mutex)
{
	if (mutex == NULL || mutex->magic != MUTEX_MAGIC) {
		return NULL;
	}

	return mutex->owner;
}

/* Internal helper functions */

/**
 * @brief Add task to mutex wait list (priority ordered)
 */
static void add_to_mutex_wait_list(mutex_t *mutex, tcb_t *task)
{
	task->next = NULL;
	task->prev = NULL;

	if (mutex->wait_list_head == NULL) {
		// First task in wait list
		mutex->wait_list_head = task;
		mutex->wait_list_tail = task;
	} else {
		// Insert in priority order (highest priority first)
		tcb_t *current = mutex->wait_list_head;
		tcb_t *prev = NULL;

		while (current != NULL && current->priority >= task->priority) {
			prev = current;
			current = current->next;
		}

		if (prev == NULL) {
			// Insert at head
			task->next = mutex->wait_list_head;
			mutex->wait_list_head->prev = task;
			mutex->wait_list_head = task;
		} else if (current == NULL) {
			// Insert at tail
			prev->next = task;
			task->prev = prev;
			mutex->wait_list_tail = task;
		} else {
			// Insert in middle
			task->next = current;
			task->prev = prev;
			prev->next = task;
			current->prev = task;
		}
	}
}

/**
 * @brief Remove highest priority task from mutex wait list
 */
static tcb_t *remove_from_mutex_wait_list(mutex_t *mutex)
{
	if (mutex->wait_list_head == NULL) {
		return NULL;
	}

	tcb_t *task = mutex->wait_list_head;

	if (task->next != NULL) {
		task->next->prev = NULL;
		mutex->wait_list_head = task->next;
	} else {
		mutex->wait_list_head = NULL;
		mutex->wait_list_tail = NULL;
	}

	task->next = NULL;
	task->prev = NULL;

	return task;
}

/**
 * @brief Implement priority inheritance
 */
static void priority_inheritance(mutex_t *mutex, tcb_t *task)
{
	if (mutex->owner != NULL && task->priority > mutex->owner->priority) {
		LOG_DBG("Priority inheritance: %s (%d) -> %s (%d)", mutex->owner->name,
			mutex->owner->priority, task->name, task->priority);

		// Elevate owner's priority
		mutex->owner->priority = task->priority;

		// TODO: Update ready queue position if owner is ready
		// This requires integration with the scheduler
	}
}

/**
 * @brief Restore original priority after mutex unlock
 */
static void priority_restore(mutex_t *mutex)
{
	if (mutex->owner != NULL && mutex->owner->priority != mutex->original_priority) {

		LOG_DBG("Priority restore: %s (%d) -> (%d)", mutex->owner->name,
			mutex->owner->priority, mutex->original_priority);

		// Restore original priority
		mutex->owner->priority = mutex->original_priority;

		// TODO: Update ready queue position if owner is ready
		// This requires integration with the scheduler
	}
}
/*
*
 * @brief Remove specific task from mutex wait list
 * @param mutex Mutex handle
 * @param task Task to remove
 * @return true if task was found and removed, false otherwise
 */
static bool remove_task_from_mutex_wait_list(mutex_t *mutex, tcb_t *task)
{
	if (mutex == NULL || task == NULL) {
		return false;
	}

	// Search for the task in the wait list
	tcb_t *current = mutex->wait_list_head;
	while (current != NULL) {
		if (current == task) {
			// Found the task, remove it
			if (task->prev != NULL) {
				task->prev->next = task->next;
			} else {
				mutex->wait_list_head = task->next;
			}

			if (task->next != NULL) {
				task->next->prev = task->prev;
			} else {
				mutex->wait_list_tail = task->prev;
			}

			// Clear the task's wait list pointers
			task->next = NULL;
			task->prev = NULL;

			return true;
		}
		current = current->next;
	}

	return false; // Task not found in wait list
}

/**
 * @brief Remove task from any mutex wait list (called by timeout handler)
 * @param task Task to remove from wait lists
 * @return true if task was found and removed
 */
bool mutex_remove_waiting_task(tcb_t *task)
{
	if (task == NULL || task->waiting_object == NULL) {
		return false;
	}

	// Check if the waiting object is a mutex
	mutex_t *mutex = (mutex_t *)task->waiting_object;
	if (mutex->magic != MUTEX_MAGIC) {
		return false; // Not a valid mutex
	}

	return remove_task_from_mutex_wait_list(mutex, task);
}