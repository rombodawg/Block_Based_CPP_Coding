@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "CONFIG_FILE=%~dp0CppCircuitLab.toolchain.txt"
if not exist "%CONFIG_FILE%" goto detect_compiler
set "MSYS2_ROOT="
set /p "MSYS2_ROOT="<"%CONFIG_FILE%"
if not defined MSYS2_ROOT goto no_compiler
if /i "%MSYS2_ROOT%"=="PATH" goto detect_compiler
if not exist "%MSYS2_ROOT%\ucrt64\bin\g++.exe" goto invalid_saved_path
set "PATH=%MSYS2_ROOT%\ucrt64\bin;%PATH%"

goto detect_compiler
:detect_compiler
where g++ >nul 2>nul
if errorlevel 1 goto no_compiler
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -Isrc tests\core_tests.cpp -o CppCircuitLabTests.exe
if errorlevel 1 goto failed
CppCircuitLabTests.exe
if errorlevel 1 goto failed
echo.
echo All core tests passed.
pause
exit /b 0
:invalid_saved_path
echo Saved MSYS2 location is no longer valid. Run Build_Cpp_Circuit_Lab.bat to update it.
pause
exit /b 1
:no_compiler
echo This test script needs g++ available through the saved MSYS2 UCRT64 path or PATH.
echo Run Build_Cpp_Circuit_Lab.bat and select a working MSYS2 toolchain first.
pause
exit /b 1
:failed
echo One or more tests failed.
pause
exit /b 1
