set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 查找工具链
find_program(ARM_GCC arm-none-eabi-gcc.exe)
get_filename_component(TOOLCHAIN_PATH ${ARM_GCC} DIRECTORY)

# 设置编译器
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/arm-none-eabi-gcc.exe)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_OBJCOPY ${TOOLCHAIN_PATH}/arm-none-eabi-objcopy.exe)
set(CMAKE_SIZE ${TOOLCHAIN_PATH}/arm-none-eabi-size.exe)

# MCU 标志
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
set(COMMON_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")

# 编译标志
set(CMAKE_C_FLAGS "${COMMON_FLAGS}" CACHE STRING "C compiler flags")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS}" CACHE STRING "ASM compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections" CACHE STRING "Linker flags")

# 寻找库和头文件设置
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)