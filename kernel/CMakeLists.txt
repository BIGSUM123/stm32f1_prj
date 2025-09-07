add_library(kernel STATIC
    init.c
    device.c
    sched.c
    rtos_core.c
    timeout.c
    mutex.c
    semaphore.c
)

target_include_directories(kernel 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Device/ST/STM32F4xx/Include
)

target_compile_definitions(kernel
    PRIVATE
        STM32F407xx
)

target_link_libraries(kernel PRIVATE
    components
    drivers
)
