#ifndef __DRIVERS_RESET_H__
#define __DRIVERS_RESET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "device.h"

/** Reset controller device configuration. */
struct reset_dt_spec {
	/** Reset controller device. */
	const struct device *dev;
	/** Reset line. */
	uint32_t id;
};

#ifdef __cplusplus
}
#endif

#endif
