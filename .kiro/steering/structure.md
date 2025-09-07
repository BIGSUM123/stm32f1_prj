# Project Structure

## Top-Level Organization

```
├── app/                    # STM32F103 application
├── app_f407/              # STM32F407 application (primary target)
├── arch/                  # Architecture-specific code
├── bootloader/            # Custom bootloader implementation
├── cmake/                 # Build system configuration
├── components/            # Reusable software components
├── drivers/               # Hardware abstraction layer drivers
├── include/               # Global header files
├── kernel/                # Core kernel implementation
└── modules/               # External libraries (STM32 HAL)
```

## Architecture Guidelines

### Application Layer (`app/`, `app_f407/`)
- Contains main application code and MCU-specific configurations
- Each app targets a specific STM32 variant
- Includes linker scripts (`.ld` files) and startup code
- CMakeLists.txt defines executable targets and dependencies

### Architecture Layer (`arch/`)
- ARM Cortex-M specific assembly code
- Vector tables, reset handlers, fault handlers
- Context switching and low-level system functions
- Shared across all applications

### Kernel Layer (`kernel/`)
- Core OS functionality: initialization, device management, scheduling
- Device abstraction and initialization system
- Entry point (`w_cstart`) that calls system init before main

### Driver Layer (`drivers/`)
- Hardware abstraction for peripherals (UART, GPIO, Flash, Clock)
- MCU-specific implementations (e.g., `uart_stm32f1.c`, `uart_stm32f4.c`)
- Common driver interface definitions

### Component Layer (`components/`)
- Higher-level software modules (CLI, logging, flash management)
- Reusable across different applications
- Well-defined APIs in `components/include/`

## File Naming Conventions

- **C files**: `snake_case.c`
- **Headers**: `snake_case.h`
- **Assembly**: `snake_case.S` (capital S for preprocessed assembly)
- **MCU-specific**: Include MCU series in filename (e.g., `uart_stm32f4.c`)

## Include Path Strategy

- Global includes: `include/` directory
- Component includes: `components/include/`
- Local includes: Relative to source file
- HAL includes: Through modules subdirectories

## Build Artifacts

- Executables: `app`, `app_f407`, `bootloader`
- Generated files: `.hex`, `.bin`, `.map` files in build directory
- Compile commands: `compile_commands.json` for IDE integration

## Key Configuration Files

- `CMakeLists.txt` - Root build configuration
- `cmake/arm-none-eabi.cmake` - Toolchain configuration
- `*.ld` - Linker scripts for memory layout
- `make.bat`/`make.sh` - Build convenience scripts

## Coding Patterns

- Device driver pattern with common interface
- Initialization system using function pointers and sections
- UART-based logging system for debugging
- Modular component architecture with clear dependencies