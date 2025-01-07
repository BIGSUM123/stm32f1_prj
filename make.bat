@echo off
setlocal enabledelayedexpansion

set BUILD_TYPE=Release
set TARGET=all

:parse_args
if "%1"=="" goto :done_parsing
if "%1"=="debug" set BUILD_TYPE=Debug
if "%1"=="release" set BUILD_TYPE=Release
if "%1"=="app" set TARGET=app
if "%1"=="boot" set TARGET=bootloader
if "%1"=="clean" goto :clean
shift
goto :parse_args

:clean
if exist build rmdir /s /q build
goto :eof

:done_parsing
if not exist build mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
ninja %TARGET%
cd ..

:eof
endlocal