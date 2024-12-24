cmake_minimum_required(VERSION 3.15)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(ARM_GCC arm-none-eabi-gcc.exe)
get_filename_component(TOOLCHAIN_PATH ${ARM_GCC} DIRECTORY)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/arm-none-eabi-objcopy.exe)
set(CMAKE_SIZE ${TOOLCHAIN_PATH}/arm-none-eabi-size.exe)

project(stm32_project C ASM)

set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(COMMON_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

add_compile_definitions(
    STM32F103x6
    USE_FULL_LL_DRIVER
)

add_subdirectory(Drivers)
add_subdirectory(components)
add_subdirectory(app)
add_subdirectory(bootloader)