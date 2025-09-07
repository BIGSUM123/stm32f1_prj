@echo off
setlocal enabledelayedexpansion

if "%1"=="" (
    echo Usage: 
    echo   addr_lookup.bat ^<address^>          - Lookup address to source
    echo   addr_lookup.bat -f ^<function^>      - Find function address
    echo   addr_lookup.bat -l ^<file:line^>     - Find line address
    echo.
    echo Examples:
    echo   addr_lookup.bat 080013D4
    echo   addr_lookup.bat -f allocate_tcb
    echo   addr_lookup.bat -l kernel/rtos_core.c:288
    exit /b 1
)

if "%1"=="-f" (
    echo === Function Address Lookup for %2 ===
    echo.
    echo --- Symbol table search ---
    arm-none-eabi-nm.exe -n .\build_F4\app_f407\app_f407 | findstr /i "%2"
    echo.
    echo --- Map file search ---
    findstr /i "%2" .\build_F4\app_f407\app_f407.map
    goto :eof
)

if "%1"=="-l" (
    echo === Line Address Lookup for %2 ===
    echo.
    echo --- Generating disassembly with source ---
    arm-none-eabi-objdump.exe -d -S .\build_F4\app_f407\app_f407 > temp_disasm.txt
    echo Disassembly saved to temp_disasm.txt
    echo Search for your file and line in the generated file
    goto :eof
)

echo.
echo === Address Lookup for %1 ===
echo.

echo --- Function and Line Info ---
arm-none-eabi-addr2line.exe -e .\build_F4\app_f407\app_f407 -f -C %1

echo.
echo --- Disassembly around address ---
arm-none-eabi-objdump.exe -d .\build_F4\app_f407\app_f407 | findstr /C:"%1"

echo.
echo --- Symbol table search ---
arm-none-eabi-nm.exe -n .\build_F4\app_f407\app_f407 | findstr /C:"%1"