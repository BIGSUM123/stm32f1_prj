set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 根据操作系统设置可执行文件后缀
if(WIN32)
    set(EXE_SUFFIX ".exe")
else()
    set(EXE_SUFFIX "")
endif()

# 查找工具链
find_program(ARM_GCC "arm-none-eabi-gcc${EXE_SUFFIX}")
get_filename_component(TOOLCHAIN_PATH ${ARM_GCC} DIRECTORY)

# 设置编译器
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/arm-none-eabi-gcc${EXE_SUFFIX}")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/arm-none-eabi-objcopy${EXE_SUFFIX}")
set(CMAKE_SIZE "${TOOLCHAIN_PATH}/arm-none-eabi-size${EXE_SUFFIX}")

# MCU 标志 (从平台配置获取)
if(DEFINED CPU_FLAGS)
    set(MCU_FLAGS ${CPU_FLAGS})
else()
    # 默认配置 (如果平台配置未加载)
    set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb")
endif()

set(COMMON_FLAGS "${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections -std=c11")

# 编译标志
set(CMAKE_C_FLAGS "${COMMON_FLAGS}" CACHE STRING "C compiler flags")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS}" CACHE STRING "ASM compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections" CACHE STRING "Linker flags")

# 修改汇编编译器的默认行为
# set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> -x assembler-with-cpp <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")

# 寻找库和头文件设置
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)