@echo off
echo First of All. You need a compiler on your Windows Device
echo Compiler: MinGW, GNU Compiler (GCC and G++)
echo.
echo if your compiler doesn't appear above, you cannot compile it.
pause

gcc -c "Common Source Modules/unicode_kit.c" -o unicode_kit.o
if not exist "Common Source Modules/LCT_decodeC.c" (
  echo LCT decoder it's not yet public for everyone
  echo So the header was not compiled
  pause
  exit
)
gcc -c "Common Source Modules/LCT_decodeC.c" -o LCT_decodeC.o

if exist "unicode_kit.o" if exist "LCT_decodeC.o" (
  g++ main.cpp unicode_kit.o LCT_decodeC.o -o "Gemdrive Giacavi 1.0a.exe" -O2 -lSDL3 -I"Common Source Modules"
) else (
  echo Application Compilation failed
)

pause
