add_executable(app_f407
    Core/Src/main.c
    # Core/Src/gpio.c
    # Core/Src/cli_commands.c
    # Core/Src/stm32ffxx_it.c
    Core/Src/system_stm32f4xx.c
    Core/Src/sysmem.c
    Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/arch/arm/reset.S
    ${CMAKE_SOURCE_DIR}/arch/arm/vector_table.S
    ${CMAKE_SOURCE_DIR}/arch/arm/fault_s.S
    ${CMAKE_SOURCE_DIR}/arch/arm/swap_helper.S
    ${CMAKE_SOURCE_DIR}/arch/arm/prep_c.c
)

target_compile_definitions(app_f407
    PRIVATE
        STM32F407xx
        USE_FULL_LL_DRIVER
)

target_include_directories(app_f407 PRIVATE
    Core/Inc
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/components/include/cli
    ${CMAKE_SOURCE_DIR}/components/include/log
)

target_link_libraries(app_f407 PRIVATE
    # drivers
    kernel
    # components
    stm32f4_module
)

# Link options
target_link_options(app_f407 PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F407VETX_FLASH.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/app_f407.map
)

# Generate hex and bin files
add_custom_command(TARGET app_f407 POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:app_f407> ${CMAKE_CURRENT_BINARY_DIR}/app_f407.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:app_f407> ${CMAKE_CURRENT_BINARY_DIR}/app_f407.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:app_f407>
)