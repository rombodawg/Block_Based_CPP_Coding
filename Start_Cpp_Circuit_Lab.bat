@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "CONFIG_FILE=%~dp0CppCircuitLab.toolchain.txt"
set "MSYS2_ROOT="

echo.
echo  C++ CIRCUIT LAB - START
echo  =======================
echo.

call :load_config
if errorlevel 1 goto build_prompt
if exist "%~dp0CppCircuitLab.exe" goto launch_app

rem The toolchain was selected previously, but the app has not been built here yet.
if /i "%MSYS2_ROOT%"=="PATH" goto build_with_path
call "%~dp0Build_Cpp_Circuit_Lab.bat" nopause "%MSYS2_ROOT%"
if errorlevel 1 goto build_failed
goto build_finished

:build_with_path
call "%~dp0Build_Cpp_Circuit_Lab.bat" nopause PATH_ONLY
if errorlevel 1 goto build_failed
goto build_finished

:build_prompt
rem First run, missing config, or a stale toolchain path: ask Build to select it.
call "%~dp0Build_Cpp_Circuit_Lab.bat" nopause
if errorlevel 1 goto build_failed
goto build_finished

:build_finished
if errorlevel 1 goto build_failed
call :load_config
if errorlevel 1 goto config_failed
if not exist "%~dp0CppCircuitLab.exe" goto build_failed

goto launch_app

:launch_app
if /i "%MSYS2_ROOT%"=="PATH" goto launch_path_only
set "CPP_CIRCUIT_LAB_MSYS2_ROOT=%MSYS2_ROOT%"
set "UCRT64_BIN=%MSYS2_ROOT%\ucrt64\bin"
set "PATH=%UCRT64_BIN%;%PATH%"
echo [TOOLCHAIN] Reusing saved MSYS2 location: "%UCRT64_BIN%\g++.exe"
goto launch_exe

:launch_path_only
set "CPP_CIRCUIT_LAB_MSYS2_ROOT="
echo [TOOLCHAIN] Reusing the compiler previously discovered through PATH.

:launch_exe
echo [LAUNCH] Starting CppCircuitLab.exe...
start "" "%~dp0CppCircuitLab.exe"
echo [READY] C++ Circuit Lab is running.
echo [NOTE] Build and Start share the same saved toolchain setting; there is no separate Run script.
echo [NOTE] To change the compiler location, run Build_Cpp_Circuit_Lab.bat again, then use Start_Cpp_Circuit_Lab.bat.
echo.
pause
exit /b 0

:load_config
set "MSYS2_ROOT="
if not exist "%CONFIG_FILE%" exit /b 1
set /p "MSYS2_ROOT="<"%CONFIG_FILE%"
if not defined MSYS2_ROOT exit /b 1
if /i "%MSYS2_ROOT%"=="PATH" goto validate_path_compiler
if exist "%MSYS2_ROOT%\ucrt64\bin\g++.exe" exit /b 0
exit /b 1

:validate_path_compiler
where g++ >nul 2>nul
if not errorlevel 1 exit /b 0
where clang++ >nul 2>nul
if not errorlevel 1 exit /b 0
where cl >nul 2>nul
if not errorlevel 1 exit /b 0
exit /b 1

:build_failed
echo.
echo [STOPPED] The application could not be built. Read the errors above.
pause
exit /b 1

:config_failed
echo.
echo [ERROR] The build finished, but no usable toolchain configuration was found.
echo Run Build_Cpp_Circuit_Lab.bat and select a valid compiler installation.
pause
exit /b 1
