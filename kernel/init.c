#include "init.h"
#include "device.h"
#include <stddef.h>

typedef struct {
    uint32_t *stack_ptr;  // 栈指针必须是 TCB 的第一个成员
} tcb_t;

tcb_t *pxCurrentTCB;      // 当前任务指针

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
			if (rc > UINT8_MAX) {
				rc = UINT8_MAX;
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

void w_start(void)
{
    sys_init_run();

    extern int main(void);

	(void)main();
}
