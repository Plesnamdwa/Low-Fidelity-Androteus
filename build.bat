@echo off
setlocal
echo ========================================================
echo   Membangun Prototype Game UI ^& Animasi (OpenGL ^& C++)
echo ========================================================

set CXX=g++
set CXXFLAGS=-std=c++17 -O3 -s -mwindows -Wall -Wno-unused-variable -static -static-libgcc -static-libstdc++ -I"C:\VSCode\Runner\MinGW\include"
set LDFLAGS=-L"C:\VSCode\Runner\MinGW\lib" -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32
set OUTPUT=game_prototype.exe

echo Mengompilasi sumber daya C++ untuk versi rilis...
%CXX% %CXXFLAGS% src\main.cpp src\Renderer2D.cpp src\UI.cpp src\Game.cpp -o %OUTPUT% %LDFLAGS%

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo   [SUKSES] Kompilasi Versi Rilis Berhasil!
    echo   File Binary : %OUTPUT%
    echo   Tipe        : Standalone Native Windows Portable
    echo ========================================================
) else (
    echo.
    echo ========================================================
    echo   [ERROR] Kompilasi Gagal! Periksa log kesalahan di atas.
    echo ========================================================
)
endlocal
