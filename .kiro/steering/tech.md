# Technology Stack

## Build System
- **CMake** (minimum version 3.20) - Primary build system
- **Ninja** - Build generator for fast compilation
- **ARM GCC Toolchain** - Cross-compilation for ARM Cortex-M

## Toolchain Requirements
- `arm-none-eabi-gcc` - C compiler for ARM bare metal
- `arm-none-eabi-objcopy` - Binary conversion utilities
- `arm-none-eabi-size` - Memory usage analysis

## Languages & Standards
- **C11** - Primary programming language
- **ARM Assembly** - Low-level system code and startup
- Chinese comments are acceptable in the codebase

## Hardware Abstraction
- **STM32 HAL/LL Drivers** - Hardware abstraction layers
- **CMSIS** - ARM Cortex Microcontroller Software Interface Standard
- Custom device driver abstraction layer

## Key Libraries & Modules
- STM32F1xx_HAL - For STM32F103 series
- STM32F4xx_HAL - For STM32F407 series  
- Custom kernel with device management
- Logging system with UART backend
- CLI (Command Line Interface) component

## Common Build Commands

### Windows (using make.bat)
```cmd
# Debug build
make.bat debug

# Release build (default)
make.bat release

# Build specific targets
make.bat app      # Build application only
make.bat boot     # Build bootloader only

# Clean build directory
make.bat clean
```

### Linux/macOS (using make.sh)
```bash
# Debug build
./make.sh debug

# Release build (default)
./make.sh release

# Build specific targets
./make.sh app     # Build application only
./make.sh boot    # Build bootloader only

# Clean build directory
./make.sh clean
```

### Direct CMake Usage
```bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

## Compiler Flags
- `-mcpu=cortex-m4 -mthumb` - ARM Cortex-M4 target
- `-Wall` - Enable warnings
- `-fdata-sections -ffunction-sections` - Optimize for size
- `-specs=nano.specs -specs=nosys.specs` - Newlib nano and no system calls
- `-Wl,--gc-sections` - Remove unused sections during linking