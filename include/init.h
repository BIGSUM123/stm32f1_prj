#ifndef __INIT_H__
#define __INIT_H__

struct device;

/**
 * @brief Initialization function for init entries.
 *
 * Init entries support both the system initialization and the device
 * APIs. Each API has its own init function signature; hence, we have a
 * union to cover both.
 */
union init_function {
	/**
	 * System initialization function.
	 *
	 * @retval 0 On success
	 * @retval -errno If init fails.
	 */
	int (*sys)(void);
	/**
	 * Device initialization function.
	 *
	 * @param dev Device instance.
	 *
	 * @retval 0 On success
	 * @retval -errno If device initialization fails.
	 */
	int (*dev)(const struct device *dev);
};

/**
 * @brief Structure to store initialization entry information.
 *
 * @internal
 * Init entries need to be defined following these rules:
 *
 * - Their name must be set using Z_INIT_ENTRY_NAME().
 * - They must be placed in a special init section, given by
 *   Z_INIT_ENTRY_SECTION().
 * - They must be aligned, e.g. using Z_DECL_ALIGN().
 *
 * See SYS_INIT_NAMED() for an example.
 * @endinternal
 */
struct init_entry {
	/** Initialization function. */
	union init_function init_fn;
	/**
	 * If the init entry belongs to a device, this fields stores a
	 * reference to it, otherwise it is set to NULL.
	 */
	union {
		const struct device *dev;
	};
};

#define INIT_ENTRY_SECTION      \
    __attribute__((section("._init_fn"), used))

#endif // __INIT_H__
