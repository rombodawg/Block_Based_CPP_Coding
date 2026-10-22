@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "CONFIG_FILE=%~dp0CppCircuitLab.toolchain.txt"
set "PAUSE_AT_END=1"
if /i "%~1"=="nopause" set "PAUSE_AT_END=0"
set "MSYS2_ROOT="

rem Start_Cpp_Circuit_Lab.bat may pass the selected toolchain as argument 2.
if /i "%~2"=="PATH_ONLY" goto detect_path
if not "%~2"=="" set "MSYS2_ROOT=%~2"
if not defined MSYS2_ROOT if defined CPP_CIRCUIT_LAB_MSYS2_ROOT set "MSYS2_ROOT=%CPP_CIRCUIT_LAB_MSYS2_ROOT%"
if defined MSYS2_ROOT goto check_msys2

echo.
echo  C++ CIRCUIT LAB - BUILD
echo  =======================
echo.
echo Enter your MSYS2 installation folder (the folder containing ucrt64\bin).
echo Press Enter to use a C++ compiler already available on PATH instead.
echo Type PATH to skip the MSYS2 location prompt and use PATH discovery.
echo.
set /p "MSYS2_ROOT=MSYS2 installation folder: "
if /i "%MSYS2_ROOT%"=="PATH" set "MSYS2_ROOT="
if defined MSYS2_ROOT goto check_msys2
goto detect_path

:check_msys2
if "%MSYS2_ROOT:~-1%"=="\" set "MSYS2_ROOT=%MSYS2_ROOT:~0,-1%"
set "UCRT64_BIN=%MSYS2_ROOT%\ucrt64\bin"
if not exist "%UCRT64_BIN%\g++.exe" (
    echo.
    echo [ERROR] No g++.exe was found at:
    echo   "%UCRT64_BIN%\g++.exe"
    echo.
    echo Enter the MSYS2 root folder, not the ucrt64\bin folder itself.
    echo You can re-run this script and enter a different location.
    if "%PAUSE_AT_END%"=="1" pause
    exit /b 1
)
rem Save the selected toolchain locally so Start can reuse it on later launches.
rem This generated file is ignored by version control and is not included in the ZIP.
>"%CONFIG_FILE%" <nul set /p "=%MSYS2_ROOT%"
set "CPP_CIRCUIT_LAB_MSYS2_ROOT=%MSYS2_ROOT%"
set "PATH=%UCRT64_BIN%;%PATH%"
echo [TOOLCHAIN] "%UCRT64_BIN%\g++.exe"
goto build_ucrt_gcc

:detect_path
set "CPP_CIRCUIT_LAB_MSYS2_ROOT="
where g++ >nul 2>nul
if not errorlevel 1 goto build_gcc
where clang++ >nul 2>nul
if not errorlevel 1 goto build_clang
where cl >nul 2>nul
if not errorlevel 1 goto build_msvc

echo.
echo [ERROR] I could not find a C++ compiler.
echo.
echo Install MSYS2 from https://www.msys2.org/ and open its UCRT64 terminal.
echo Run:
echo   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
echo.
echo Then rerun this script and enter your MSYS2 installation folder.
echo You can also use g++, clang++, or cl if it is already available on PATH.
echo.
if "%PAUSE_AT_END%"=="1" pause
exit /b 1

:build_ucrt_gcc
echo [BUILD] Building the native Windows app with the configured UCRT64 compiler...
"%UCRT64_BIN%\g++.exe" -std=c++17 -O2 -Wall -Wextra -pedantic -mwindows src\cpp_circuit_lab.cpp -o CppCircuitLab.exe -lcomdlg32 -lshell32 -lgdi32 -luser32
if errorlevel 1 goto failed
goto success

:build_gcc
call :save_path_config
for /f "delims=" %%C in ('where g++') do if not defined GCC_PATH set "GCC_PATH=%%C"
echo [TOOLCHAIN] %GCC_PATH%
echo [BUILD] Building the native Windows app with g++...
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -mwindows src\cpp_circuit_lab.cpp -o CppCircuitLab.exe -lcomdlg32 -lshell32 -lgdi32 -luser32
if errorlevel 1 goto failed
goto success

:build_clang
call :save_path_config
for /f "delims=" %%C in ('where clang++') do if not defined CLANG_PATH set "CLANG_PATH=%%C"
echo [TOOLCHAIN] %CLANG_PATH%
echo [BUILD] Building the native Windows app with clang++...
clang++ -std=c++17 -O2 -Wall -Wextra -pedantic -mwindows src\cpp_circuit_lab.cpp -o CppCircuitLab.exe -lcomdlg32 -lshell32 -lgdi32 -luser32
if errorlevel 1 goto failed
goto success

:build_msvc
call :save_path_config
for /f "delims=" %%C in ('where cl') do if not defined MSVC_PATH set "MSVC_PATH=%%C"
echo [TOOLCHAIN] %MSVC_PATH%
echo [BUILD] Building the native Windows app with MSVC...
cl /nologo /EHsc /std:c++17 /utf-8 /O2 src\cpp_circuit_lab.cpp /Fe:CppCircuitLab.exe /link /SUBSYSTEM:WINDOWS comdlg32.lib shell32.lib gdi32.lib user32.lib
if errorlevel 1 goto failed
if exist cpp_circuit_lab.obj del cpp_circuit_lab.obj
goto success

:save_path_config
>"%CONFIG_FILE%" <nul set /p "=PATH"
exit /b 0

:success
echo.
echo [SUCCESS] Built CppCircuitLab.exe
echo [CONFIG] Toolchain choice saved for Start_Cpp_Circuit_Lab.bat.
echo [NEXT] Use Start_Cpp_Circuit_Lab.bat to build if needed and open the app.
echo.
if "%PAUSE_AT_END%"=="1" pause
exit /b 0

:failed
echo.
echo [ERROR] Build failed. Read the compiler's first error above.
echo.
if "%PAUSE_AT_END%"=="1" pause
exit /b 1
