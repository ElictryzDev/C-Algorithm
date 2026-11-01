@echo off
title C-Algorithm Interactive Runner (ANSI C)
echo === C-Algorithm: Program Derleniyor ve Baslatiliyor (ANSI C)... ===
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cl.exe /nologo /TC /O1 "C:\Users\ASUS TUF GAMING\Desktop\C-Algorithm\projects\_build_temp.c" /Fe:"C:\Users\ASUS TUF GAMING\Desktop\C-Algorithm\projects\_build_temp.exe"
if %errorlevel% equ 0 (
    echo === PROGRAM CALISTIRILIYOR ===
    echo.
    "C:\Users\ASUS TUF GAMING\Desktop\C-Algorithm\projects\_build_temp.exe"
    echo.
    echo === Program sonlandi. Pencereyi kapatabilirsiniz. ===
) else (
    echo [HATA] Derleme basarisiz oldu! Yukaridaki derleyici ciktisini inceleyin.
)
