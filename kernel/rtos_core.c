#include "rtos.h"
#include "kernel.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "stm32f4xx_ll_utils.h"

/* ARM CMSIS includes for intrinsic functions */
#if defined(__ARM_ARCH) || defined(STM32F407xx) || defined(STM32F103xx)
#include "stm32f407xx.h" // STM32F407 specific definitions (includes core_cm4.h)
#else
/* Fallback definitions for non-ARM platforms */
#define __disable_irq()
#define __enable_irq()
#define __WFI()
#define SCB_ICSR_PENDSVSET_Msk 0
typedef struct {
	uint32_t ICSR;
} SCB_Type;
SCB_Type *SCB = (SCB_Type *)0;
#endif

/* RTOS Core State */
typedef struct {
	bool initialized;          // RTOS是否已初始化
	bool running;              // RTOS是否正在运行
	uint32_t tick_count;       // 系统滴答计数
	uint32_t context_switches; // 上下文切换次数
	uint32_t lock_count;       // 调度锁计数
} rtos_core_t;

static rtos_core_t rtos_core = {0};

/* Task Pool */
static tcb_t task_pool[RTOS_MAX_TASKS];
static bool task_pool_used[RTOS_MAX_TASKS] = {false};

/* Ready Queues */
typedef struct {
	tcb_t *head;
	tcb_t *tail;
	uint32_t count;
} ready_queue_t;

static ready_queue_t ready_queues[RTOS_MAX_PRIORITIES];
static uint8_t ready_bitmap = 0;

/* Current Task Pointer (for compatibility) */
tcb_t *pxCurrentTCB = NULL;

/* Idle Task */
static tcb_t idle_task_tcb;
static uint32_t idle_task_stack[RTOS_IDLE_STACK_SIZE]; // 256 bytes stack for idle task

/* Forward Declarations */
static void idle_task_function(void *parameter);
static tcb_t *allocate_tcb(void);
static void free_tcb(tcb_t *tcb);
static void init_task_stack(tcb_t *task);
static uint8_t find_highest_priority(void);
void add_to_ready_queue(tcb_t *task);
void remove_from_ready_queue(tcb_t *task);

/**
 * @brief Initialize RTOS core
 */
rtos_error_t rtos_init(void)
{
	if (rtos_core.initialized) {
		return RTOS_OK; // Already initialized
	}

	// Initialize ready queues
	for (int i = 0; i < RTOS_MAX_PRIORITIES; i++) {
		ready_queues[i].head = NULL;
		ready_queues[i].tail = NULL;
		ready_queues[i].count = 0;
	}

	// Initialize task pool
	memset(task_pool, 0, sizeof(task_pool));
	memset(task_pool_used, false, sizeof(task_pool_used));

	// Create idle task
	idle_task_tcb.entry = idle_task_function;
	idle_task_tcb.parameter = NULL;
	strcpy(idle_task_tcb.name, "IDLE");
	idle_task_tcb.priority = RTOS_PRIORITY_IDLE;
	idle_task_tcb.stack_base = &idle_task_stack[RTOS_IDLE_STACK_SIZE];
	idle_task_tcb.stack_size = sizeof(idle_task_stack);
	idle_task_tcb.time_slice = RTOS_TIME_SLICE_TICKS;
	idle_task_tcb.time_remaining = RTOS_TIME_SLICE_TICKS;
	idle_task_tcb.stack_canary = RTOS_STACK_CANARY;

	// Initialize idle task stack with proper context
	init_task_stack(&idle_task_tcb);

	// Add idle task to ready queue
	add_to_ready_queue(&idle_task_tcb);

	// Set current task to idle task
	pxCurrentTCB = &idle_task_tcb;

	rtos_core.initialized = true;
	rtos_core.running = false;
	rtos_core.tick_count = 0;
	rtos_core.context_switches = 0;
	rtos_core.lock_count = 0;

	return RTOS_OK;
}

/**
 * @brief Start RTOS scheduler
 */
rtos_error_t rtos_start(void)
{
	if (!rtos_core.initialized) {
		return RTOS_ERROR;
	}

	if (rtos_core.running) {
		return RTOS_OK; // Already running
	}

	// Find the first task to run
	uint8_t highest_priority = find_highest_priority();
	if (highest_priority >= RTOS_MAX_PRIORITIES) {
		LOG_DBG("1");
		return RTOS_ERROR; // No tasks to run
	}

	// Get the first task from the highest priority queue
	ready_queue_t *queue = &ready_queues[highest_priority];
	if (queue->head == NULL) {
		LOG_DBG("2");
		// This should not happen if bitmap is correct
		return RTOS_ERROR; // No tasks in queue
	}

	// Set current task
	pxCurrentTCB = queue->head;

	// Debug: Check if task and stack are valid
	if (pxCurrentTCB == NULL) {
		LOG_DBG("ERROR: pxCurrentTCB is NULL!");
		return RTOS_ERROR;
	}

	if (pxCurrentTCB->stack_ptr == NULL) {
		LOG_DBG("ERROR: stack_ptr is NULL!");
		return RTOS_ERROR;
	}

	LOG_DBG("Starting task: %s, stack_ptr: 0x%08X", pxCurrentTCB->name,
		(uint32_t)pxCurrentTCB->stack_ptr);

	// Debug: Print stack frame content
	uint32_t *sp = (uint32_t *)pxCurrentTCB->stack_ptr;
	LOG_DBG("Stack frame: R4-R7: 0x%08X 0x%08X 0x%08X 0x%08X", sp[0], sp[1], sp[2], sp[3]);
	LOG_DBG("Stack frame: R8-R11: 0x%08X 0x%08X 0x%08X 0x%08X", sp[4], sp[5], sp[6], sp[7]);
	LOG_DBG("Hardware frame start: 0x%08X", (uint32_t)(sp + 8));

	// SysTick is already configured by HAL_Init() in main()
	// No need to reconfigure it here

	// Mark RTOS as running
	rtos_core.running = true;

	// Start first task (will be implemented in assembly)
	extern void start_first_thread(void);
	start_first_thread();

	// Should never reach here
	return RTOS_ERROR;
}

/**
 * @brief Check if RTOS is running
 */
bool rtos_is_running(void)
{
	return rtos_core.running;
}

/**
 * @brief Check if RTOS is initialized
 */
bool rtos_is_initialized(void)
{
	return rtos_core.initialized;
}

/**
 * @brief Get ready bitmap for debugging
 */
uint8_t rtos_get_ready_bitmap(void)
{
	return ready_bitmap;
}

/**
 * @brief Get current system tick count
 */
uint32_t rtos_get_tick_count(void)
{
	return rtos_core.tick_count;
}

/**
 * @brief Convert milliseconds to ticks
 */
uint32_t rtos_ms_to_ticks(uint32_t milliseconds)
{
	return (milliseconds * RTOS_TICK_RATE_HZ) / 1000;
}

/**
 * @brief Convert ticks to milliseconds
 */
uint32_t rtos_ticks_to_ms(uint32_t ticks)
{
	return (ticks * 1000) / RTOS_TICK_RATE_HZ;
}

/**
 * @brief Scheduler lock
 */
void scheduler_lock(void)
{
	__disable_irq();
	rtos_core.lock_count++;
	__enable_irq();
}

/**
 * @brief Scheduler unlock
 */
void scheduler_unlock(void)
{
	__disable_irq();
	if (rtos_core.lock_count > 0) {
		rtos_core.lock_count--;
	}
	__enable_irq();
}

/**
 * @brief Check if scheduler is locked
 */
bool scheduler_is_locked(void)
{
	return (rtos_core.lock_count > 0);
}

/**
 * @brief Idle task function
 */
static void idle_task_function(void *parameter)
{
	(void)parameter; // Unused parameter

	while (1) {
		// Yield to allow other tasks to run
		// Don't use __WFI() here as it might prevent task switching
		// task_yield();

		// Small delay to prevent busy waiting
		// for (volatile int i = 0; i < 1000; i++)
		// 	;
		// LOG_DBG("idle");
		__WFI();
	}
}

/**
 * @brief Allocate a TCB from the pool
 */
static tcb_t *allocate_tcb(void)
{
	for (int i = 0; i < RTOS_MAX_TASKS; i++) {
		if (!task_pool_used[i]) {
			task_pool_used[i] = true;
			memset(&task_pool[i], 0, sizeof(tcb_t));
			return &task_pool[i];
		}
	}
	return NULL; // No free TCB available
}

/**
 * @brief Free a TCB back to the pool
 */
static void free_tcb(tcb_t *tcb)
{
	if (tcb == NULL) {
		return;
	}

	for (int i = 0; i < RTOS_MAX_TASKS; i++) {
		if (&task_pool[i] == tcb) {
			task_pool_used[i] = false;
			memset(tcb, 0, sizeof(tcb_t));
			break;
		}
	}
}

/**
 * @brief Initialize task stack with proper context and canary protection
 */
static void init_task_stack(tcb_t *task)
{
	if (task == NULL || task->stack_base == NULL || task->entry == NULL) {
		return;
	}

	// Calculate stack layout
	uint32_t words = task->stack_size / sizeof(uint32_t);
	uint32_t *const bottom_word = task->stack_base - words; // 最低地址元素

	// Fill entire stack with pattern for debugging
	for (uint32_t i = 0; i < words; i++) {
		bottom_word[i] = RTOS_STACK_FILL_PATTERN;
	}

	// Set canary value at stack bottom (lowest address)
	*bottom_word = RTOS_STACK_CANARY;
	task->stack_canary = RTOS_STACK_CANARY;

	// Initialize stack pointer to top of stack (highest address)
	uint32_t *sp = task->stack_base;          // 指向 top+1
	sp = (uint32_t *)((uintptr_t)sp & ~0x7U); // 8 字节对齐

	// Reserve space for hardware-saved registers (8 words)
	// ARM Cortex-M hardware automatically saves these on exception entry
	sp -= 8;

	// Initialize hardware frame (saved by hardware on exception entry)
	// These are in the order they are pushed by hardware (high to low address)
	sp[7] = 0x01000000U;               // xPSR (Thumb mode bit set)
	sp[6] = (uint32_t)task->entry;     // PC (task entry point)
	sp[5] = 0xFFFFFFFDU;               // LR (EXC_RETURN for thread mode with PSP)
	sp[4] = 0x00000000U;               // R12
	sp[3] = 0x00000000U;               // R3
	sp[2] = 0x00000000U;               // R2
	sp[1] = 0x00000000U;               // R1
	sp[0] = (uint32_t)task->parameter; // R0 (task parameter)

	// Reserve space for software-saved registers (8 words: R4-R11)
	// These are saved/restored by our context switch code
	sp -= 8;

	// Initialize software frame (saved by our PendSV handler)
	sp[7] = 0x00000000U; // R11
	sp[6] = 0x00000000U; // R10
	sp[5] = 0x00000000U; // R9
	sp[4] = 0x00000000U; // R8
	sp[3] = 0x00000000U; // R7
	sp[2] = 0x00000000U; // R6
	sp[1] = 0x00000000U; // R5
	sp[0] = 0x00000000U; // R4

	// Update task's stack pointer to point to the top of the initialized frame
	task->stack_ptr = sp;
}

/**
 * @brief Find highest priority with ready tasks
 */
static uint8_t find_highest_priority(void)
{
	if (ready_bitmap == 0) {
		return RTOS_PRIORITY_IDLE; // Only idle task
	}

	// Find highest set bit (highest priority)
	// Start from highest priority and work down
	for (int i = RTOS_MAX_PRIORITIES - 1; i >= 0; i--) {
		if (ready_bitmap & (1 << i)) {
			return i;
		}
	}

	// Should never reach here if ready_bitmap != 0
	return RTOS_PRIORITY_IDLE;
}

/**
 * @brief Add task to ready queue
 */
void add_to_ready_queue(tcb_t *task)
{
	if (task == NULL || task->priority >= RTOS_MAX_PRIORITIES) {
		return;
	}

	ready_queue_t *queue = &ready_queues[task->priority];

	task->next = NULL;
	task->prev = queue->tail;

	if (queue->tail != NULL) {
		queue->tail->next = task;
	} else {
		queue->head = task;
	}

	queue->tail = task;
	queue->count++;

	// Set priority bit
	ready_bitmap |= (1 << task->priority);

	task->state = TASK_STATE_READY;
}

/**
 * @brief Remove task from ready queue
 */
void remove_from_ready_queue(tcb_t *task)
{
	if (task == NULL || task->priority >= RTOS_MAX_PRIORITIES) {
		return;
	}

	ready_queue_t *queue = &ready_queues[task->priority];

	if (task->prev != NULL) {
		task->prev->next = task->next;
	} else {
		queue->head = task->next;
	}

	if (task->next != NULL) {
		task->next->prev = task->prev;
	} else {
		queue->tail = task->prev;
	}

	queue->count--;

	// Clear priority bit if no more tasks at this priority
	if (queue->count == 0) {
		ready_bitmap &= ~(1 << task->priority);
	}

	task->next = NULL;
	task->prev = NULL;
}

/* Static task stacks for testing */
static uint32_t task_stack_1[128] __attribute__((aligned(8))); // 512 bytes
static uint32_t task_stack_2[256] __attribute__((aligned(8))); // 1024 bytes
static uint32_t task_stack_3[256] __attribute__((aligned(8))); // 512 bytes
static uint32_t task_stack_4[256] __attribute__((aligned(8))); // 512 bytes
static uint32_t task_stack_5[256] __attribute__((aligned(8))); // 512 bytes
static bool static_stacks_used[5] = {false, false, false, false, false};

/**
 * @brief Allocate a static stack for testing (temporary solution)
 */
static uint32_t *allocate_static_stack(uint32_t size, uint32_t *actual_size)
{
	// try 512-byte stacks
	if (size <= 512 && !static_stacks_used[0]) {
		LOG_DBG("use stack 0");
		static_stacks_used[0] = true;
		*actual_size = sizeof(task_stack_1);
		return task_stack_1;
	}

	// Then try to allocate 1024-byte stack first (for CLI task)
	if (size <= 1024) {
		for (int i = 1; i < 5; i++) {
			if (!static_stacks_used[i]) {
				static_stacks_used[i] = true;
				LOG_DBG("use stack %d", i);
				switch (i) {
				case 1:
					*actual_size = sizeof(task_stack_2);
					return task_stack_2;
				case 2:
					*actual_size = sizeof(task_stack_3);
					return task_stack_3;
				case 3:
					*actual_size = sizeof(task_stack_4);
					return task_stack_4;
				case 4:
					*actual_size = sizeof(task_stack_5);
					return task_stack_5;
				}
			}
		}
	}

	return NULL; // No available stack
}

/**
 * @brief Create a new task
 */
task_handle_t task_create(const char *name, task_func_t entry, void *parameter, uint32_t stack_size,
			  uint8_t priority)
{
	if (!rtos_core.initialized || entry == NULL || priority >= RTOS_MAX_PRIORITIES) {
		return NULL;
	}

	// Allocate TCB
	tcb_t *task = allocate_tcb();
	if (task == NULL) {
		return NULL;
	}

	// Allocate stack (use static allocation for now)
	uint32_t actual_size;
	uint32_t *stack = allocate_static_stack(stack_size, &actual_size);
	if (stack == NULL) {
		free_tcb(task);
		return NULL;
	}

	// Initialize task
	task->entry = entry;
	task->parameter = parameter;
	strncpy(task->name, name ? name : "UNNAMED", sizeof(task->name) - 1);
	task->name[sizeof(task->name) - 1] = '\0';
	task->priority = priority;

	// Stack grows downward, so stack_base points to the highest address
	task->stack_base = &stack[(actual_size / sizeof(uint32_t))];
	task->stack_size = actual_size;
	task->time_slice = RTOS_TIME_SLICE_TICKS;
	task->time_remaining = RTOS_TIME_SLICE_TICKS;
	task->stack_canary = RTOS_STACK_CANARY;

	// Initialize stack
	init_task_stack(task);

	// Add to ready queue
	__disable_irq();
	add_to_ready_queue(task);
	__enable_irq();

	return task;
}

/**
 * @brief Delete a task
 */
rtos_error_t task_delete(task_handle_t task)
{
	if (task == NULL) {
		return RTOS_INVALID_PARAM;
	}

	__disable_irq();

	// Remove from ready queue if ready
	if (task->state == TASK_STATE_READY) {
		remove_from_ready_queue(task);
	}

	// Mark as deleted
	task->state = TASK_STATE_DELETED;

	// Free stack memory
	if (task->stack_base != NULL) {
		// Calculate original stack pointer
		uint32_t *original_stack =
			task->stack_base - (task->stack_size / sizeof(uint32_t)) + 1;
		free(original_stack);
	}

	// Free TCB
	free_tcb(task);

	__enable_irq();

	// If deleting current task, trigger scheduler
	if (task == pxCurrentTCB) {
		pxCurrentTCB = NULL;
		// Trigger PendSV for context switch
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}

	return RTOS_OK;
}

/**
 * @brief Suspend a task
 */
rtos_error_t task_suspend(task_handle_t task)
{
	if (task == NULL) {
		return RTOS_INVALID_PARAM;
	}

	__disable_irq();

	if (task->state == TASK_STATE_READY) {
		remove_from_ready_queue(task);
		task->state = TASK_STATE_SUSPENDED;
	}

	__enable_irq();

	// If suspending current task, trigger scheduler
	if (task == pxCurrentTCB) {
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}

	return RTOS_OK;
}

/**
 * @brief Resume a suspended task
 */
rtos_error_t task_resume(task_handle_t task)
{
	if (task == NULL) {
		return RTOS_INVALID_PARAM;
	}

	__disable_irq();

	if (task->state == TASK_STATE_SUSPENDED) {
		add_to_ready_queue(task);
	}

	__enable_irq();

	return RTOS_OK;
}

/**
 * @brief Yield current task
 */
void task_yield(void)
{
	if (rtos_core.running && pxCurrentTCB != NULL) {
		// Reset time slice
		pxCurrentTCB->time_remaining = pxCurrentTCB->time_slice;
		// Trigger PendSV for context switch
		SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	}
}

/**
 * @brief Delay current task for specified ticks
 */
void task_delay(uint32_t ticks)
{
	if (!rtos_core.running || pxCurrentTCB == NULL || ticks == 0) {
		return;
	}

	__disable_irq();

	// Set wake time
	pxCurrentTCB->wake_time = rtos_core.tick_count + ticks;

	// LOG_DBG("Task delay: current_tick=%lu, wake_time=%lu, ticks=%lu", rtos_core.tick_count,
	// 	pxCurrentTCB->wake_time, ticks);

	// Remove from ready queue
	remove_from_ready_queue(pxCurrentTCB);
	pxCurrentTCB->state = TASK_STATE_BLOCKED;

	// LOG_DBG("Task blocked: state=%d", pxCurrentTCB->state);

	__enable_irq();

	// Trigger context switch
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Delay current task for specified milliseconds
 */
void task_delay_ms(uint32_t milliseconds)
{
	uint32_t ticks = rtos_ms_to_ticks(milliseconds);
	task_delay(ticks);
}

/**
 * @brief Get current task handle
 */
task_handle_t task_get_current(void)
{
	return pxCurrentTCB;
}

/**
 * @brief Get task name
 */
const char *task_get_name(task_handle_t task)
{
	if (task == NULL) {
		return "INVALID";
	}
	return task->name;
}

/**
 * @brief Get task priority
 */
uint8_t task_get_priority(task_handle_t task)
{
	if (task == NULL) {
		return 0;
	}
	return task->priority;
}

/**
 * @brief Get task state
 */
task_state_t task_get_state(task_handle_t task)
{
	if (task == NULL) {
		return TASK_STATE_DELETED;
	}
	return (task_state_t)task->state;
}

/**
 * @brief Process delayed tasks and wake up expired ones
 */
static void process_delayed_tasks(void)
{
	// static task_state_t led_task_state = 0xFF; // Initialize to invalid state
	// static uint32_t debug_counter = 0;

	// debug_counter++;

	// Simple implementation: scan all tasks for delayed ones
	for (int i = 0; i < RTOS_MAX_TASKS; i++) {
		if (task_pool_used[i]) {
			tcb_t *task = &task_pool[i];

			// // Debug every 100 calls or when state changes
			// if (led_task_state != task->state || (debug_counter % 100) == 0) {
			// 	LOG_DBG("Task[%d] state=%d, wake_time=%lu, current_tick=%lu", i,
			// 		task->state, task->wake_time, rtos_core.tick_count);
			// 	led_task_state = task->state;
			// }

			if (task->state == TASK_STATE_BLOCKED) {
				// Check for task_delay() timeout
				if (task->waiting_object == NULL &&
				    (int32_t)(rtos_core.tick_count - task->wake_time) >= 0) {
					// Task delay expired, move to ready queue
					add_to_ready_queue(task);
				}
				// Check for mutex/semaphore timeout
				else if (task->waiting_object != NULL && task->wait_timeout != 0 &&
					 (int32_t)(rtos_core.tick_count - task->wait_timeout) >=
						 0) {
					// Waiting timeout expired

					// Remove from wait list first (before clearing
					// waiting_object)
					mutex_remove_waiting_task(task);

					// Clear waiting information
					task->wait_result = RTOS_TIMEOUT;
					task->waiting_object = NULL;

					// Move back to ready queue
					add_to_ready_queue(task);
				}
			}
		}
	}
}

void systick_hardle(void)
{
	static uint32_t last_tick = 0;
	// Increment system tick count
	rtos_core.tick_count++;
	if (rtos_core.tick_count - last_tick >= 100) {
		last_tick = rtos_core.tick_count;
		// LOG_DBG("TICKS %d", rtos_core.tick_count);
	}

	// Update current task time slice
	if (rtos_core.running && pxCurrentTCB != NULL) {
		if (pxCurrentTCB->time_remaining > 0) {
			pxCurrentTCB->time_remaining--;
		}

		// Update task run time statistics
		pxCurrentTCB->run_time++;
	}

	// Process delayed tasks
	process_delayed_tasks();

	// Check if we need to reschedule
	if (rtos_core.running && !rtos_core.lock_count) {
		// Check if current task's time slice expired or higher priority task is ready
		uint8_t highest_priority = find_highest_priority();

		bool need_reschedule = false;

		if (pxCurrentTCB != NULL) {
			// Time slice expired for current task
			if (pxCurrentTCB->time_remaining == 0 &&
			    ready_queues[pxCurrentTCB->priority].count > 1) {
				need_reschedule = true;
			}

			// Higher priority task became ready
			if (highest_priority > pxCurrentTCB->priority) {
				need_reschedule = true;
			}
		}

		if (need_reschedule) {
			// Trigger PendSV for context switch
			SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
		}
	}
}

/**
 * @brief Scheduler function (called from assembly)
 */
void w_scheduler(void)
{
	if (!rtos_core.running || rtos_core.lock_count > 0) {
		return; // Scheduler not running or locked
	}

	// Find highest priority ready task
	uint8_t highest_priority = find_highest_priority();
	ready_queue_t *queue = &ready_queues[highest_priority];

	if (queue->head == NULL) {
		// Should not happen, idle task should always be ready
		pxCurrentTCB = &idle_task_tcb;
		return;
	}

	tcb_t *next_task = queue->head;

	// Time slice rotation for same priority tasks
	if (queue->count > 1 && next_task == pxCurrentTCB && pxCurrentTCB->time_remaining == 0) {
		// Move current task to end of queue
		remove_from_ready_queue(pxCurrentTCB);
		pxCurrentTCB->time_remaining = pxCurrentTCB->time_slice;
		add_to_ready_queue(pxCurrentTCB);
		next_task = queue->head;
	}

	// Update current task
	if (next_task != pxCurrentTCB) {
		if (pxCurrentTCB != NULL && pxCurrentTCB->state == TASK_STATE_RUNNING) {
			// Only set to READY if the task is currently RUNNING
			// Don't change state if task is BLOCKED, SUSPENDED, etc.
			pxCurrentTCB->state = TASK_STATE_READY;
		}
		pxCurrentTCB = next_task;
		pxCurrentTCB->state = TASK_STATE_RUNNING;
		rtos_core.context_switches++;
	}
}