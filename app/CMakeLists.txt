add_executable(app
    Core/Src/main.c
    Core/Src/gpio.c
    Core/Src/cli_commands.c
    Core/Src/stm32f1xx_it.c
    Core/Src/system_stm32f1xx.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/arch/arm/reset.s
    ${CMAKE_SOURCE_DIR}/arch/arm/vector_table.s
    ${CMAKE_SOURCE_DIR}/arch/arm/fault_s.s
)

target_include_directories(app PRIVATE
    Core/Inc
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/components/include/cli
    ${CMAKE_SOURCE_DIR}/components/include/log
    ${CMAKE_SOURCE_DIR}/components/include/flash
)

target_link_libraries(app PRIVATE
    drivers
    kernel
    components
    stm32f1_module
)

# Link options
target_link_options(app PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/app.map
)

# Generate hex and bin files
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:app> ${CMAKE_CURRENT_BINARY_DIR}/app.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:app> ${CMAKE_CURRENT_BINARY_DIR}/app.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:app>
)