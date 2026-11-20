@echo off
REM Corre los 4 ejemplos del video. Presiona Enter para ejecutar cada comando.
chcp 65001 >nul
set PATH=C:\Program Files\JetBrains\CLion 2025.2\bin\mingw\bin;%PATH%
cd /d "%~dp0.."
g++ -std=c++17 -O2 -o rv32sim src/main.cpp src/cpu.cpp src/decode.cpp src/disasm.cpp src/loader.cpp
cls
rv32sim.exe tests\bin\riscvtest.bin --script demo\1_riscvtest.txt --pause
rv32sim.exe tests\bin\quicksort-O2.bin --script demo\2_quicksort.txt --pause
rv32sim.exe tests\bin\symmetric-O2.bin --script demo\3_simetrico.txt --pause
rv32sim.exe tests\bin\hello.bin --script demo\4_ecall.txt --pause
echo.
echo Fin de los ejemplos.
pause
