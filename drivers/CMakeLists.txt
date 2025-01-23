add_library(drivers STATIC)

add_subdirectory(flash)
add_subdirectory(gpio)
add_subdirectory(uart)

target_include_directories(drivers 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)