set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Set toolchain paths
set(TOOLCHAIN_PATH "D:/Arm GNU Toolchain arm-none-eabi/13.3 rel1/bin")
set(TOOLCHAIN_PREFIX "${TOOLCHAIN_PATH}/arm-none-eabi-")

# Set compilers
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc.exe")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc.exe")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}objcopy.exe")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}size.exe")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU flags
set(CPU "-mcpu=cortex-m3")
set(FPU "")
set(FLOAT_ABI "")

set(CMAKE_C_FLAGS "${CPU} ${FPU} ${FLOAT_ABI} -mthumb -fdata-sections -ffunction-sections" CACHE STRING "C compiler flags")
set(CMAKE_ASM_FLAGS "${CPU} ${FPU} ${FLOAT_ABI} -mthumb" CACHE STRING "ASM compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "${CPU} ${FPU} ${FLOAT_ABI} -mthumb -specs=nosys.specs -Wl,--gc-sections" CACHE STRING "Linker flags")