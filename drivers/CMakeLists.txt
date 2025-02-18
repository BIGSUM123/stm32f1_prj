add_library(drivers OBJECT)

target_include_directories(drivers 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(flash)
add_subdirectory(gpio)
add_subdirectory(uart)
