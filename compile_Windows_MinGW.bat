@echo off
echo First of All. You need a compiler on your Windows Device
echo Compiler: MinGW, GNU Compiler (GCC and G++)
echo.
echo if your compiler doesn't appear above, you cannot compile it.
pause

gcc -c common_source_modules/unicode_kit.c -o unicode_kit.o
gcc -c common_source_modules/styleButtonFlow.c -o styleButtonFlow.o
gcc -c common_source_modules/codeTokenType.c -o codeTokenType.o
if not exist common_source_modules/LCT_decodeC.c (
  echo LCT decoder it's not yet public for everyone
  echo So the header was not compiled
  pause
  exit
)
gcc -c common_source_modules/LCT_decodeC.c -o LCT_decodeC.o

if exist "unicode_kit.o" if exist "LCT_decodeC.o" (
  g++ main.cpp unicode_kit.o styleButtonFlow.o codeTokenType.o LCT_decodeC.o -o "Gemdrive Giacavi 1.0a.exe" -O2 -lSDL3 -Icommon_source_modules
) else (
  echo Application Compilation failed
)

pause
