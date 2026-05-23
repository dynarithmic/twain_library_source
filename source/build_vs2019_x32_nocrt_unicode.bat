setlocal

echo Using:
cmake --version
echo.

rem ============================================================
rem DTWAIN Build Script
rem
rem Edit the values below if desired.
rem ============================================================

rem If using automatic Boost download (you can use any directory name other than D:/BoostDeps):
set "BOOST_CACHE_ROOT=D:/BoostDeps"
set "EXISTING_BOOST_ROOT="

rem If using an existing Boost install, use this instead:
rem set "EXISTING_BOOST_ROOT=D:/boost_1_90_0"

set "SOURCE_DIR=%~dp0."
for %%I in ("%SOURCE_DIR%") do set "SOURCE_DIR=%%~fI"
set "BUILD_DIR=%SOURCE_DIR%\build-vs2019-x32-nocrt_unicode"

set BOOST_CACHE_ROOT=D:/BoostDeps

cmake -S "%SOURCE_DIR%" ^
      -B "%BUILD_DIR%" ^
      -G "Visual Studio 16 2019" ^
      -A Win32 ^
      -DDTWAIN_BUILD_UNICODE=ON ^
      -DDTWAIN_ENABLE_LOGCALLSTACK=ON ^
      -DDTWAIN_USE_DYNAMIC_CRT=OFF ^
      "-DTWAIN_BOOST_CACHE_ROOT=%BOOST_CACHE_ROOT%" ^
      "-DTWAIN_EXISTING_BOOST_ROOT=%EXISTING_BOOST_ROOT%" ^
      -DTWAIN_AUTO_DOWNLOAD_BOOST=ON

if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --config MinSizeRel -- /m
cmake --build "%BUILD_DIR%" --config Debug -- /m

endlocal

