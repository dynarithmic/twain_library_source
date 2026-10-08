echo Using:
cmake --version
echo.

@echo off

cmake --preset vs2022-x64-crt-unicode-cpp20
if errorlevel 1 exit /b 1

cmake --build --preset vs2022-x64-crt-unicode-cpp20-release -- /m
if errorlevel 1 exit /b 1

cmake --build --preset vs2022-x64-crt-unicode-cpp20-debug -- /m
if errorlevel 1 exit /b 1

endlocal
