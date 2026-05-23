setlocal

echo Using:
cmake --version
echo.

rem ============================================================
rem DTWAIN Build Script
rem
rem Edit the values below if desired.
rem ============================================================

set "SOURCE_DIR=%~dp0."
for %%I in ("%SOURCE_DIR%") do set "SOURCE_DIR=%%~fI"

rem If using automatic Boost download (you can use any directory name other than D:/BoostDeps):
set "BOOST_CACHE_ROOT=%SOURCE_DIR%\BoostDeps"
set "EXISTING_BOOST_ROOT="

set "BUILD_DIR=%SOURCE_DIR%\build-vs2019-x64-crt_ansi"

rem If using an existing Boost install, use this instead (for example, if boost is installed in D:/boost_1_90_0):
rem set "EXISTING_BOOST_ROOT=D:/boost_1_90_0"

cmake -S "%SOURCE_DIR%" ^
      -B "%BUILD_DIR%" ^
      -G "Visual Studio 16 2019" ^
      -A x64 ^
      -DDTWAIN_BUILD_UNICODE=OFF ^
      -DDTWAIN_ENABLE_LOGCALLSTACK=ON ^
      -DDTWAIN_USE_DYNAMIC_CRT=ON ^
      "-DTWAIN_BOOST_CACHE_ROOT=%BOOST_CACHE_ROOT%" ^
      "-DTWAIN_EXISTING_BOOST_ROOT=%EXISTING_BOOST_ROOT%" ^
      -DTWAIN_AUTO_DOWNLOAD_BOOST=ON

if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --config MinSizeRel -- /m
cmake --build "%BUILD_DIR%" --config Debug -- /m

endlocal

