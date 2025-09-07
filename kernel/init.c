#include "init.h"
#include "device.h"
#include "log.h"
#include "rtos.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ARM CMSIS includes for intrinsic functions */
#if defined(__ARM_ARCH) || defined(STM32F407xx) || defined(STM32F103xx)
#include "stm32f407xx.h"
#else
#define __WFI()
#endif

static int do_device_init(const struct init_entry *entry)
{
    const struct device *dev = entry->dev;
	int rc = 0;

	if (entry->init_fn.dev != NULL) {
		rc = entry->init_fn.dev(dev);
		/* Mark device initialized. If initialization
		 * failed, record the error condition.
		 */
		if (rc != 0) {
			if (rc < 0) {
				rc = -rc;
			}
			if (rc > 255) {
				rc = 255;
			}
			dev->state->init_res = rc;
		}
	}

	dev->state->initialized = true;

	// if (rc == 0) {
	// 	/* Run automatic device runtime enablement */
	// 	(void)pm_device_runtime_auto_enable(dev);
	// }

	return rc;
}

static void sys_init_run()
{
	extern const struct init_entry _init_fn_start[];
	extern const struct init_entry _init_fn_end[];

	const struct init_entry *entry = NULL;
    // int result = -1;

    for (entry = _init_fn_start; entry < _init_fn_end; entry++) {
        const struct device *dev = entry->dev;

        if (dev != NULL) {
			do_device_init(entry);
        } else {
			entry->init_fn.sys();
		}
    }
}

void w_cstart(void)
{
    // Initialize system devices first
    sys_init_run();

    // Initialize RTOS
    rtos_error_t result = rtos_init();
    if (result != RTOS_OK) {
        // Handle RTOS initialization error
        // For now, just continue without RTOS
        // TODO: Add error logging here
    }

    extern int main(void);

    // Call main function
    (void)main();
    
    // If main returns and RTOS is initialized, start scheduler
    if (result == RTOS_OK) {
        rtos_start();
    }
    
    // Should never reach here if RTOS is running
    while (1) {
        __WFI();
    }
}
