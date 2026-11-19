#!/bin/sh
# Ensambla/compila los programas de prueba a binarios crudos RV32I (requiere clang + lld).
set -e
cd "$(dirname "$0")"
mkdir -p bin
AS="llvm-mc --triple=riscv32 -mattr=-c --filetype=obj"
for s in riscvtest instrs hello; do
  $AS $s.s -o bin/$s.o
  ld.lld -T link.ld -e 0 bin/$s.o -o bin/$s.elf
  llvm-objcopy -O binary bin/$s.elf bin/$s.bin
done
$AS crt0.s -o bin/crt0.o
for c in quicksort symmetric; do
  for opt in O0 O2; do
    clang --target=riscv32 -march=rv32i -mabi=ilp32 -$opt -ffreestanding -nostdlib -c $c.c -o bin/$c.o
    ld.lld -T link.ld bin/crt0.o bin/$c.o -o bin/$c-$opt.elf
    llvm-objcopy -O binary bin/$c-$opt.elf bin/$c-$opt.bin
  done
done
rm -f bin/*.o
ls -l bin
