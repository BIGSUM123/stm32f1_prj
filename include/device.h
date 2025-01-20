#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include <stdbool.h>

#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)

#define INIT_ENTRY_SECTION      \
    __attribute__((section("._init_fn"), used))

#define DEVICE_SECTION_NAME         \
    __attribute__((section("._device_obj"), used))

#define DEVICE_NAME_GET(dev_id) 

#define DEVICE_INIT(name_, pm_, data_, config_, api_, state_)   \
    {                                                      \
        .name = name_,                              \
        .config = config_,                              \
        .api = api_,                        \
        .state = state_,                        \
        .data = data_,                      \
    }

/**
 * @brief 
 * 
 */
#define DEVICE_BASE_DEFINE(dev_id, name, pm, data, config, level, \
                prio, api, state)                           \
                                                \
    DEVICE_SECTION_NAME                     \
    static device_t CONCAT(_device_, dev_id) =               \
    DEVICE_INIT(name, pm, data, config, api, state)

/**
 * @brief 
 * 
 */
#define DEVICE_INIT_ENTRY_DEFINE(_dev_id, _init_fn, \
                level, prio)                \
                                                \
    const static INIT_ENTRY_SECTION                 \
    init_entry_t CONCAT(__init_fn_, _dev_id) = {         \
        .init_fn = _init_fn,                    \
    }

/**
 * @brief 
 * 
 */
#define DEVICE_DEFINE(dev_id, name, \
                      init_fn, pm, \
                      data, config, level, prio, \
                      api, state) \
                                                        \
    DEVICE_BASE_DEFINE(dev_id, name, pm, data, config, level,    \
		        prio, api, state);      \
                                                        \
    DEVICE_INIT_ENTRY_DEFINE(dev_id,            \
                init_fn, level, prio)                 \

typedef struct init_entry {
    int (*init_fn)(void);
} init_entry_t;

typedef struct device_state {
    uint8_t init_res;
    bool initialized : 1;
} device_state_t;

typedef struct device {
    const char *name;
    const void *config;
    const void *api;
    device_state_t *state;
    void *data;
} device_t;

#endif // __DEVICE_H__