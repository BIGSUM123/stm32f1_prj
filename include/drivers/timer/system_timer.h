#ifndef __SYSTEM_TIMER_H__
#define __SYSTEM_TIMER_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Announce time progress to the kernel
 *
 * Informs the kernel that the specified number of ticks have elapsed
 * since the last call to sys_clock_announce() (or system startup for
 * the first call).  The timer driver is expected to delivery these
 * announcements as close as practical (subject to hardware and
 * latency limitations) to tick boundaries.
 *
 * @param ticks Elapsed time, in ticks
 */
void sys_clock_announce(int32_t ticks);

#ifdef __cplusplus
}
#endif

#endif
