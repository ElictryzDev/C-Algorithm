@echo off
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VCVARS%" >nul 2>&1
cd /d "%~dp0"
cl.exe /nologo /std:c++17 /O2 /MD /EHsc /utf-8 ^
    /Isrc /Isrc/vendor/imgui /Isrc/vendor/imgui/backends ^
    src\main.cpp src\Core\ProjectManager.cpp src\UI\Workspaces.cpp ^
    src\vendor\imgui\imgui.cpp src\vendor\imgui\imgui_draw.cpp ^
    src\vendor\imgui\imgui_tables.cpp src\vendor\imgui\imgui_widgets.cpp ^
    src\vendor\imgui\backends\imgui_impl_win32.cpp ^
    src\vendor\imgui\backends\imgui_impl_dx11.cpp ^
    d3d11.lib d3dcompiler.lib dxgi.lib user32.lib gdi32.lib shell32.lib ^
    /Fe:C-Algorithm.exe
