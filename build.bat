@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

rem --- C++ arac setini bul ----------------------------------------------------
rem Once vswhere denenir, ama yalnizca Build Tools kurulu bir makinede sonuc bos
rem doner (-products * ile -requires birlikte saglanan bir kurulum yoktur) ve bu,
rem sorunsuz calisabilecek bir derlemeyi bozardi. Asagidaki dizin taramasi yedektir.
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
    echo [!] No MSVC toolset found. Install "Desktop development with C++" from
    echo     https://visualstudio.microsoft.com/downloads/
    exit /b 1
)

echo [*] Toolset: %VSPATH%
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul || (echo [!] vcvars64 failed & exit /b 1)

set IMGUI=third_party\imgui
if not exist "%IMGUI%\imgui.h" (
    echo [!] third_party\imgui bulunamadi.
    exit /b 1
)

rem Gomulu varliklari yeniden uret: res\fonts ve res\tengri-logo.png tek kaynak.
rem Bu adim birakilirsa onceki sürumden kalan .cpp derlenir ve degisiklikler
rem sessizce yoksayilir.
echo [*] Embedding fonts and logo ...
powershell -NoProfile -ExecutionPolicy Bypass -File tools\make_font_data.ps1 || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\make_logo_data.ps1 -WhiteArtwork || exit /b 1

if not exist build mkdir build
if not exist build\obj\core mkdir build\obj\core
if not exist build\obj\preview mkdir build\obj\preview
if not exist build\obj\app mkdir build\obj\app

set FLAGS=/nologo /std:c++17 /O2 /MT /EHsc /utf-8 /W4 /MP ^
   /DNDEBUG /DUNICODE /D_UNICODE /DIMGUI_DEFINE_MATH_OPERATORS ^
   /I src /I %IMGUI% /I %IMGUI%\backends

set LIBS=d3d11.lib dxgi.lib d3dcompiler.lib dwmapi.lib user32.lib gdi32.lib shell32.lib windowscodecs.lib ole32.lib

rem Cekirdek bir kez derlenir; iki yurutulebilir ayni .obj setini linkler,
rem fark yalnizca içerik modülüdür (app::content()).
set CORE=src\main.cpp src\app\shell.cpp ^
 src\gui\theme.cpp src\gui\fx.cpp src\gui\widgets.cpp src\gui\icons.cpp src\gui\brand_icons.cpp ^
 src\gui\logo.cpp src\gui\logo_data.cpp src\gui\font_data.cpp ^
 %IMGUI%\imgui.cpp %IMGUI%\imgui_draw.cpp %IMGUI%\imgui_tables.cpp %IMGUI%\imgui_widgets.cpp ^
 %IMGUI%\backends\imgui_impl_win32.cpp %IMGUI%\backends\imgui_impl_dx11.cpp

set CORE_NAMES=main shell theme fx widgets icons brand_icons logo logo_data font_data ^
 imgui imgui_draw imgui_tables imgui_widgets imgui_impl_win32 imgui_impl_dx11

echo [*] Compiling core ...
cl %FLAGS% /c %CORE% /Fobuild\obj\core\
if errorlevel 1 (
    echo [!] Build failed.
    exit /b 1
)

set OBJ=
for %%F in (%CORE_NAMES%) do set OBJ=!OBJ! build\obj\core\%%F.obj

echo [*] Building build\TengriApp.exe  (senin uygulaman: src\app\my_app.cpp) ...
cl %FLAGS% src\app\my_app.cpp !OBJ! ^
   /Fobuild\obj\app\ /link /SUBSYSTEM:WINDOWS %LIBS% /OUT:build\TengriApp.exe
if errorlevel 1 (
    echo [!] Build failed.
    exit /b 1
)

echo [*] Building build\TengriThemePreview.exe  (tema vitrini: src\demo.cpp) ...
cl %FLAGS% src\demo.cpp !OBJ! ^
   /Fobuild\obj\preview\ /link /SUBSYSTEM:WINDOWS %LIBS% /OUT:build\TengriThemePreview.exe
if errorlevel 1 (
    echo [!] Build failed.
    exit /b 1
)

echo [+] Done: build\TengriApp.exe, build\TengriThemePreview.exe
