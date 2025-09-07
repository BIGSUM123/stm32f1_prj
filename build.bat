@echo off
REM STM32 RTOS 构建脚本 (Windows版本)

setlocal enabledelayedexpansion

REM 默认参数
set PLATFORM=%1
set BUILD_TYPE=%2
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

REM 检查参数
if "%PLATFORM%"=="" (
    echo Usage: build.bat [PLATFORM] [BUILD_TYPE]
    echo.
    echo Supported platforms:
    echo   STM32F103
    echo   STM32F407
    echo   STM32F746
    echo   all
    echo.
    echo Build types: Debug, Release, MinSizeRel
    echo.
    echo Examples:
    echo   build.bat STM32F407
    echo   build.bat STM32F103 Debug
    echo   build.bat all
    exit /b 1
)

REM 检查工具
cmake --version >nul 2>&1
if errorlevel 1 (
    echo Error: CMake not found
    exit /b 1
)

ninja --version >nul 2>&1
if errorlevel 1 (
    echo Error: Ninja not found
    exit /b 1
)

REM 构建函数
if "%PLATFORM%"=="all" (
    call :build_platform STM32F103 %BUILD_TYPE%
    call :build_platform STM32F407 %BUILD_TYPE%
    call :build_platform STM32F746 %BUILD_TYPE%
) else (
    call :build_platform %PLATFORM% %BUILD_TYPE%
)

echo.
echo === Build Summary ===
for /d %%d in (build_*) do (
    if exist "%%d\*.elf" (
        echo ✅ %%d: Success
    ) else (
        echo ❌ %%d: Failed
    )
)

exit /b 0

:build_platform
set PLAT=%1
set TYPE=%2

echo.
echo === Building %PLAT% (%TYPE%) ===

REM 创建构建目录
set BUILD_DIR=build_%PLAT:STM32=%
set BUILD_DIR=%BUILD_DIR:~0,-2%
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM CMake配置
cd %BUILD_DIR%
cmake -G Ninja -DPLATFORM=%PLAT% -DCMAKE_BUILD_TYPE=%TYPE% -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
if errorlevel 1 (
    echo Error: CMake configuration failed for %PLAT%
    cd ..
    exit /b 1
)

REM 构建
ninja
if errorlevel 1 (
    echo Error: Build failed for %PLAT%
    cd ..
    exit /b 1
)

cd ..
echo ✅ %PLAT% build completed!
exit /b 0