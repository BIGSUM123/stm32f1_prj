add_executable(bootloader
    Core/Src/main.c
    Core/Src/gpio.c
    Core/Src/boot.c
    Core/Src/cli_commands.c
    Core/Src/stm32f1xx_it.c
    Core/Src/system_stm32f1xx.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/arch/arm/reset.S
    ${CMAKE_SOURCE_DIR}/arch/arm/vector_table.S
    ${CMAKE_SOURCE_DIR}/arch/arm/fault_s.S
    ${CMAKE_SOURCE_DIR}/arch/arm/swap_helper.S
)

target_include_directories(bootloader PRIVATE
    Core/Inc
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/components/include/cli
    ${CMAKE_SOURCE_DIR}/components/include/log
    ${CMAKE_SOURCE_DIR}/components/include/flash
)

target_link_libraries(bootloader PRIVATE
    components
    stm32f1_module
    drivers
    kernel
)

target_compile_definitions(bootloader
    PRIVATE
        STM32F103x6
)

target_link_options(bootloader PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F103C8Tx_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bootloader.map
)

add_custom_command(TARGET bootloader POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:bootloader> ${CMAKE_CURRENT_BINARY_DIR}/bootloader.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:bootloader> ${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:bootloader>
)