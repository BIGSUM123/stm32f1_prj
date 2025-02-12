add_library(kernel STATIC
    init.c
    device.c
    sched.c
)

target_include_directories(kernel 
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)