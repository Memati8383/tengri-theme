@echo off
rem Bu betik tests\ altindadir; kaynak yollari depo kokune goredir.
cd /d "%~dp0.."

rem --- C++ arac setini bul (build.bat ile ayni iki adimli yedek) ---------------
set "VSPATH="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
)
if not defined VSPATH (
    for /d %%D in ("%ProgramFiles(x86)%\Microsoft Visual Studio\2022\*" "%ProgramFiles%\Microsoft Visual Studio\2022\*") do (
        if exist "%%~fD\VC\Auxiliary\Build\vcvars64.bat" set "VSPATH=%%~fD"
    )
)
if not defined VSPATH (
    echo [!] No MSVC toolset found.
    exit /b 1
)
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul || (echo [!] vcvars64 failed & exit /b 1)

if not exist build mkdir build
if not exist build\testobj mkdir build\testobj

echo [*] Building tests ...
cl /nologo /std:c++17 /O2 /MT /EHsc /utf-8 /W4 ^
   /DUNICODE /D_UNICODE ^
   tests\hit_test.cpp ^
   /Fobuild\testobj\ /Febuild\hit_test.exe
if errorlevel 1 (
    echo [!] Test build failed.
    exit /b 1
)

echo [*] Running build\hit_test.exe ...
build\hit_test.exe
set RC=%ERRORLEVEL%
if "%RC%"=="0" echo [+] Tests passed.
if not "%RC%"=="0" echo [!] Tests failed.
exit /b %RC%
