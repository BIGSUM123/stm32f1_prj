#include "device.h"
#include "sys/iterable_sections.h"
#include <string.h>

extern const struct init_entry _init_fn_start[];
extern const struct init_entry _init_fn_end[];

int device_init()
{
    const struct init_entry *entry;

    for (entry = _init_fn_start; entry < _init_fn_end; entry++) {
        entry->init_fn();
    }

    return 0;
}

const struct device *device_get_binding(const char *name)
{
    /* A null string identifies no device.  So does an empty
	 * string.
	 */
	if ((name == NULL) || (name[0] == '\0')) {
		return NULL;
	}

    STRUCT_SECTION_FOREACH(device, dev) {
        if (dev->name == name) {
			return dev;
		}
    }

    STRUCT_SECTION_FOREACH(device, dev) {
		if (strcmp(name, dev->name) == 0) {
			return dev;
		}
	}

    return NULL;
}