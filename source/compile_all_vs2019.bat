call build_vs2019_x32_crt_ansi.bat
if errorlevel 1 exit /b 1

call build_vs2019_x32_crt_unicode.bat
if errorlevel 1 exit /b 1

call build_vs2019_x32_nocrt_ansi.bat
if errorlevel 1 exit /b 1

call build_vs2019_x32_nocrt_unicode.bat
if errorlevel 1 exit /b 1

call build_vs2019_x64_crt_ansi.bat
if errorlevel 1 exit /b 1

call build_vs2019_x64_crt_unicode.bat
if errorlevel 1 exit /b 1

call build_vs2019_x64_nocrt_ansi.bat
if errorlevel 1 exit /b 1

call build_vs2019_x64_nocrt_unicode.bat
if errorlevel 1 exit /b 1
