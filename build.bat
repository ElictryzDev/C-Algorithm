@echo off
setlocal
echo =========================================================================
echo   C-ALGORITHM NATIVE IDE v1.0 - DIRECTX 11 & VISUAL SCRIPTING DERLEME
echo =========================================================================

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS%" (
    echo [HATA] Visual Studio 2022 vcvars64.bat bulunamadi!
    pause
    exit /b 1
)

call "%VCVARS%" >nul 2>&1

echo [1/2] C-Algorithm C++ bilesenleri derleniyor...
cd /d "%~dp0"

cl.exe /nologo /std:c++17 /O2 /MD /EHsc ^
    /Isrc /Isrc/vendor/imgui /Isrc/vendor/imgui/backends ^
    src\main.cpp src\Core\ProjectManager.cpp src\UI\Workspaces.cpp ^
    src\vendor\imgui\imgui.cpp src\vendor\imgui\imgui_draw.cpp ^
    src\vendor\imgui\imgui_tables.cpp src\vendor\imgui\imgui_widgets.cpp ^
    src\vendor\imgui\backends\imgui_impl_win32.cpp ^
    src\vendor\imgui\backends\imgui_impl_dx11.cpp ^
    d3d11.lib d3dcompiler.lib dxgi.lib user32.lib gdi32.lib ^
    /Fe:C-Algorithm.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo =========================================================================
    echo   [BASARILI] C-Algorithm.exe olusturuldu!
    echo =========================================================================
    echo.
    echo [2/2] C-Algorithm IDE Calistiriliyor...
    start C-Algorithm.exe
) else (
    echo.
    echo [HATA] Derleme sirasinda hata olustu!
)

pause
