echo Using:
cmake --version
echo.

@echo off

cmake --preset vs2026-x32-crt-ansi
if errorlevel 1 exit /b 1

cmake --build --preset vs2026-x32-crt-ansi-release -- /m
if errorlevel 1 exit /b 1

cmake --build --preset vs2026-x32-crt-ansi-debug -- /m
if errorlevel 1 exit /b 1

endlocal

