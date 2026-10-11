@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cl.exe /nologo /std:c++17 /O1 /EHsc "C:\Users\ASUS TUF GAMING\Desktop\C-Algorithm\projects\_build_temp.cpp" /Fe:"C:\Users\ASUS TUF GAMING\Desktop\C-Algorithm\projects\_build_temp.exe"
