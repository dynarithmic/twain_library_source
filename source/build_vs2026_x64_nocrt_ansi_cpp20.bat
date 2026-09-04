echo Using:
cmake --version
echo.

@echo off

cmake --preset vs2026-x64-nocrt-ansi-cpp20
if errorlevel 1 exit /b 1

cmake --build --preset vs2026-x64-nocrt-ansi-cpp20-release -- /m
if errorlevel 1 exit /b 1

cmake --build --preset vs2026-x64-nocrt-ansi-cpp20-debug -- /m
if errorlevel 1 exit /b 1

endlocal
