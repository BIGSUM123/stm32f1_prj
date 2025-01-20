#include "device.h"
#include <string.h>

extern const device_t _device_start[];
extern const device_t _device_end[];

const device_t *device_get_binding(const char *name)
{
    const device_t *dev;

    for (dev = _device_start; dev < _device_end; dev++) {
        if (strcmp(dev->name, name) == 0) {
            return dev;
        }
    }

    return NULL;
}