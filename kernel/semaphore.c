/**
 * @file semaphore.c
 * @brief Semaphore implementation for RTOS
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
#define RTOS_MAX_SEMAPHORES         16

/* Static semaphore pool */
static semaphore_t semaphore_pool[RTOS_MAX_SEMAPHORES];
static bool semaphore_pool_used[RTOS_MAX_SEMAPHORES];

/* Internal function declarations */
static void add_to_semaphore_wait_list(semaphore_t *sem, tcb_t *task);
static tcb_t* remove_from_semaphore_wait_list(semaphore_t *sem);
static bool remove_task_from_semaphore_wait_list(semaphore_t *sem, tcb_t *task);

/**
 * @brief Create a binary semaphore
 * @param name Semaphore name (optional, can be NULL)
 * @param initial_count Initial count (0 or 1 for binary semaphore)
 * @return Semaphore handle or NULL if failed
 */
semaphore_handle_t sem_create_binary(const char *name, uint32_t initial_count)
{
    // Binary semaphore is just a counting semaphore with max_count = 1
    return sem_create_counting(name, 1, initial_count > 0 ? 1 : 0);
}

/**
 * @brief Create a counting semaphore
 * @param name Semaphore name (optional, can be NULL)
 * @param max_count Maximum count value
 * @param initial_count Initial count value
 * @return Semaphore handle or NULL if failed
 */
semaphore_handle_t sem_create_counting(const char *name, uint32_t max_count, uint32_t initial_count)
{
    if (max_count == 0 || initial_count > max_count) {
        return NULL;
    }

    __disable_irq();

    // Find free semaphore slot
    semaphore_t *sem = NULL;
    for (int i = 0; i < RTOS_MAX_SEMAPHORES; i++) {
        if (!semaphore_pool_used[i]) {
            sem = &semaphore_pool[i];
            semaphore_pool_used[i] = true;
            break;
        }
    }

    if (sem == NULL) {
        __enable_irq();
        LOG_DBG("Semaphore create failed: no free slots");
        return NULL;
    }

    // Initialize semaphore
    memset(sem, 0, sizeof(semaphore_t));
    if (name != NULL) {
        strncpy(sem->name, name, sizeof(sem->name) - 1);
        sem->name[sizeof(sem->name) - 1] = '\0';
    } else {
        strcpy(sem->name, "UNNAMED");
    }

    sem->count = initial_count;
    sem->max_count = max_count;
    sem->wait_list_head = NULL;
    sem->wait_list_tail = NULL;
    sem->is_binary = (max_count == 1);
    sem->magic = SEMAPHORE_MAGIC;

    __enable_irq();

    LOG_DBG("Semaphore created: %s (max=%lu, initial=%lu)", 
        sem->name, max_count, initial_count);

    return sem;
}

/**
 * @brief Delete a semaphore
 * @param sem Semaphore handle
 * @return RTOS_OK if successful
 */
rtos_error_t sem_delete(semaphore_handle_t sem)
{
    if (sem == NULL || sem->magic != SEMAPHORE_MAGIC) {
        return RTOS_INVALID_PARAM;
    }

    __disable_irq();

    // Wake up all waiting tasks with error
    while (sem->wait_list_head != NULL) {
        tcb_t *task = remove_from_semaphore_wait_list(sem);
        if (task != NULL) {
            task->wait_result = RTOS_ERROR;
            task->waiting_object = NULL;
            add_to_ready_queue(task);
        }
    }

    // Mark semaphore as free
    sem->magic = 0;
    for (int i = 0; i < RTOS_MAX_SEMAPHORES; i++) {
        if (&semaphore_pool[i] == sem) {
            semaphore_pool_used[i] = false;
            break;
        }
    }

    __enable_irq();

    LOG_DBG("Semaphore deleted: %s", sem->name);
    return RTOS_OK;
}

/**
 * @brief Wait on a semaphore (P operation, acquire)
 * @param sem Semaphore handle
 * @param timeout Timeout in ticks (RTOS_WAIT_FOREVER for infinite wait)
 * @return RTOS_OK if successful
 */
rtos_error_t sem_wait(semaphore_handle_t sem, uint32_t timeout)
{
    if (sem == NULL || sem->magic != SEMAPHORE_MAGIC) {
        return RTOS_INVALID_PARAM;
    }

    if (!rtos_is_running() || pxCurrentTCB == NULL) {
        return RTOS_ERROR;
    }

    __disable_irq();

    // Check if semaphore is available
    if (sem->count > 0) {
        // Semaphore available, decrement and return
        sem->count--;
        __enable_irq();
        LOG_DBG("Semaphore acquired by %s: %s (count=%lu)", 
            pxCurrentTCB->name, sem->name, sem->count);
        return RTOS_OK;
    }

    // Semaphore not available
    if (timeout == RTOS_NO_WAIT) {
        __enable_irq();
        return RTOS_TIMEOUT;
    }

    // Set up waiting information
    pxCurrentTCB->waiting_object = sem;
    pxCurrentTCB->wait_timeout = (timeout == RTOS_WAIT_FOREVER) ? 0 : 
                                (rtos_get_tick_count() + timeout);
    pxCurrentTCB->wait_result = RTOS_OK;

    // IMPORTANT: Remove from ready queue BEFORE adding to wait list
    remove_from_ready_queue(pxCurrentTCB);
    pxCurrentTCB->state = TASK_STATE_BLOCKED;

    // Add to semaphore wait list
    add_to_semaphore_wait_list(sem, pxCurrentTCB);

    __enable_irq();

    LOG_DBG("Task %s waiting on semaphore: %s", pxCurrentTCB->name, sem->name);

    // Trigger context switch
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

    // When we return here, we either got the semaphore or timed out
    return pxCurrentTCB->wait_result;
}

/**
 * @brief Post to a semaphore (V operation, release)
 * @param sem Semaphore handle
 * @return RTOS_OK if successful
 */
rtos_error_t sem_post(semaphore_handle_t sem)
{
    if (sem == NULL || sem->magic != SEMAPHORE_MAGIC) {
        return RTOS_INVALID_PARAM;
    }

    __disable_irq();

    // Check if there are waiting tasks
    tcb_t *waiting_task = remove_from_semaphore_wait_list(sem);
    if (waiting_task != NULL) {
        // Wake up the waiting task (don't increment count)
        waiting_task->waiting_object = NULL;
        waiting_task->wait_result = RTOS_OK;
        add_to_ready_queue(waiting_task);

        __enable_irq();

        LOG_DBG("Semaphore transferred to %s: %s", waiting_task->name, sem->name);

        // Trigger context switch if needed
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    } else {
        // No waiting tasks, increment count if not at maximum
        if (sem->count < sem->max_count) {
            sem->count++;
            __enable_irq();
            LOG_DBG("Semaphore posted: %s (count=%lu)", sem->name, sem->count);
        } else {
            __enable_irq();
            LOG_DBG("Semaphore post failed: %s already at max count (%lu)", 
                sem->name, sem->max_count);
            return RTOS_ERROR; // Semaphore overflow
        }
    }

    return RTOS_OK;
}

/**
 * @brief Get current semaphore count
 * @param sem Semaphore handle
 * @return Current count or 0 if invalid
 */
uint32_t sem_get_count(semaphore_handle_t sem)
{
    if (sem == NULL || sem->magic != SEMAPHORE_MAGIC) {
        return 0;
    }

    return sem->count;
}

/* Internal helper functions */

/**
 * @brief Add task to semaphore wait list (FIFO order for fairness)
 */
static void add_to_semaphore_wait_list(semaphore_t *sem, tcb_t *task)
{
    task->next = NULL;
    task->prev = NULL;

    if (sem->wait_list_head == NULL) {
        // First task in wait list
        sem->wait_list_head = task;
        sem->wait_list_tail = task;
    } else {
        // Add to tail (FIFO order)
        sem->wait_list_tail->next = task;
        task->prev = sem->wait_list_tail;
        sem->wait_list_tail = task;
    }
}

/**
 * @brief Remove first task from semaphore wait list
 */
static tcb_t* remove_from_semaphore_wait_list(semaphore_t *sem)
{
    if (sem->wait_list_head == NULL) {
        return NULL;
    }

    tcb_t *task = sem->wait_list_head;

    if (task->next != NULL) {
        task->next->prev = NULL;
        sem->wait_list_head = task->next;
    } else {
        sem->wait_list_head = NULL;
        sem->wait_list_tail = NULL;
    }

    task->next = NULL;
    task->prev = NULL;

    return task;
}

/**
 * @brief Remove specific task from semaphore wait list
 */
static bool remove_task_from_semaphore_wait_list(semaphore_t *sem, tcb_t *task)
{
    if (sem == NULL || task == NULL) {
        return false;
    }

    // Search for the task in the wait list
    tcb_t *current = sem->wait_list_head;
    while (current != NULL) {
        if (current == task) {
            // Found the task, remove it
            if (task->prev != NULL) {
                task->prev->next = task->next;
            } else {
                sem->wait_list_head = task->next;
            }

            if (task->next != NULL) {
                task->next->prev = task->prev;
            } else {
                sem->wait_list_tail = task->prev;
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
 * @brief Remove task from any semaphore wait list (called by timeout handler)
 * @param task Task to remove from wait lists
 * @return true if task was found and removed
 */
bool semaphore_remove_waiting_task(tcb_t *task)
{
    if (task == NULL || task->waiting_object == NULL) {
        return false;
    }

    // Check if the waiting object is a semaphore
    semaphore_t *sem = (semaphore_t *)task->waiting_object;
    if (sem->magic != SEMAPHORE_MAGIC) {
        return false; // Not a valid semaphore
    }

    return remove_task_from_semaphore_wait_list(sem, task);
}