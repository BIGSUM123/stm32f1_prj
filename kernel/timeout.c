#include "drivers/timer/system_timer.h"
#include "log.h"

void sys_clock_announce(int32_t ticks)
{
    static int32_t cnt = 0;
    cnt++;
	// 	k_spinlock_key_t key = k_spin_lock(&timeout_lock);

	// 	/* We release the lock around the callbacks below, so on SMP
	// 	 * systems someone might be already running the loop.  Don't
	// 	 * race (which will cause parallel execution of "sequential"
	// 	 * timeouts and confuse apps), just increment the tick count
	// 	 * and return.
	// 	 */
	// 	if (IS_ENABLED(CONFIG_SMP) && (announce_remaining != 0)) {
	// 		announce_remaining += ticks;
	// 		k_spin_unlock(&timeout_lock, key);
	// 		return;
	// 	}

	// 	announce_remaining = ticks;

	// 	struct _timeout *t;

	// 	for (t = first();
	// 	     (t != NULL) && (t->dticks <= announce_remaining);
	// 	     t = first()) {
	// 		int dt = t->dticks;

	// 		curr_tick += dt;
	// 		t->dticks = 0;
	// 		remove_timeout(t);

	// 		k_spin_unlock(&timeout_lock, key);
	// 		t->fn(t);
	// 		key = k_spin_lock(&timeout_lock);
	// 		announce_remaining -= dt;
	// 	}

	// 	if (t != NULL) {
	// 		t->dticks -= announce_remaining;
	// 	}

	// 	curr_tick += announce_remaining;
	// 	announce_remaining = 0;

	// 	sys_clock_set_timeout(next_timeout(), false);

	// 	k_spin_unlock(&timeout_lock, key);

	// #ifdef CONFIG_TIMESLICING
	// 	z_time_slice();
	// #endif /* CONFIG_TIMESLICING */
	extern void systick_hardle();
	systick_hardle();
    // if (cnt == 1000) {
	//     LOG_DBG("TICKS PAST");
    //     cnt = 0;
    // }
}