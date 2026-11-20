# rv32sim — Simulador del ISA RISC-V (RV32I)

CS3051 Arquitectura de Computadores 2026-II — Tarea 5.

Simulador de consola, escrito en **C++17**, que carga un programa RISC-V ya
compilado (binario crudo o ELF) y permite ejecutarlo instrucción por
instrucción, inspeccionando el PC, los 32 registros y la memoria.

## Compilación

Requiere un compilador C++17 (g++, clang++ o MSVC) y CMake ≥ 3.10.

```sh
cmake -S . -B build
cmake --build build
```

Sin CMake también basta con:

```sh
g++ -std=c++17 -O2 -o rv32sim src/*.cpp
```

## Uso

```sh
./build/rv32sim programa.bin          # consola interactiva
./build/rv32sim --run programa.bin    # ejecuta todo y muestra los registros
```

Ejemplo de sesión:

```
$ ./build/rv32sim tests/bin/riscvtest.bin
"tests/bin/riscvtest.bin" cargado a memoria (binario crudo, 84 bytes en 0x00000000-0x00000053, entry = 0x00000000).
> step
[0x00000000] 00500113  li sp, 5
      x2 (sp): 0x00000000 -> 0x00000005
> regs sp x3
sp = 0x00000005  (5)
x3 = 0x00000000  (0)
> run
El programa terminó: bucle infinito en 0x00000050 (salto a sí mismo).
18 instrucciones ejecutadas.
> mem 100 103
Memoria (0x00000064-0x00000067): 19 00 00 00
> exit
See you next time...
Program exited with code 0.
```

### Comandos

| Comando | Descripción |
|---|---|
| `step [n]` (`s`) | Ejecuta `n` instrucciones (1 por defecto). Muestra cada instrucción desensamblada y qué registro/memoria cambió. |
| `run [max]` (`c`) | Ejecuta hasta que el programa termine, llegue a un breakpoint o haya un error. |
| `pc` | Muestra el PC y la instrucción a la que apunta. |
| `regs [r ...]` (`r`) | Todos los registros, o solo los indicados (`x5`, `t0`, `sp`, `a0`, `pc`, ...). |
| `set <reg> <valor>` | Cambia un registro o el PC. |
| `mem <inicio> [fin]` (`m`) | Muestra bytes de memoria; para rangos grandes, en formato hexdump con ASCII. |
| `memw <inicio> [n]` | Muestra `n` palabras de 32 bits (leídas en little-endian). |
| `poke <addr> <valor>` | Escribe una palabra de 32 bits en memoria. |
| `disasm [addr] [n]` (`d`) | Desensambla `n` instrucciones (por defecto desde el PC). |
| `break <addr>` (`b`), `delete <addr>`, `breaks` | Breakpoints. |
| `info` | Instrucciones ejecutadas y estado. |
| `reset` | Recarga el programa y reinicia registros y memoria. |
| `help`, `exit` (`q`) | Ayuda / salir. |

Los números aceptan decimal (`100`) o hexadecimal (`0x64`).

### Modo demo

`--script archivo` ejecuta los comandos de un archivo de texto (uno por línea;
las líneas con `#` se muestran como comentario). Con `--pause` muestra cada
comando y lo ejecuta al presionar Enter, útil para presentar:

```sh
./build/rv32sim tests/bin/riscvtest.bin --script demo/1_riscvtest.txt --pause
```

En Windows, `demo\demo.bat` compila el simulador y corre los cuatro ejemplos
seguidos.

## Diseño

```
src/
  memory.hpp   memoria plana de 32 bits, little-endian, dispersa por páginas de 4 KiB
  decode.*     decodificación de la instrucción a una estructura Instr (op, rd, rs1, rs2, imm)
  cpu.*        estado arquitectural (PC, x0-x31, memoria) y ejecución de cada instrucción; ecalls
  disasm.*     desensamblador (usa el mismo decodificador)
  loader.*     carga de binarios crudos (a 0x00000000) y ELF32
  main.cpp     consola interactiva
```

Cada `step` hace el ciclo fetch → decode → execute → writeback:

1. **Fetch**: lee la palabra de 32 bits en `mem[PC]` (little-endian).
2. **Decode**: según el opcode y `funct3`/`funct7` determina la instrucción y
   arma el inmediato con extensión de signo según su formato (I, S, B, U, J).
3. **Execute**: aplica la operación sobre los registros/memoria. `x0` se
   ignora como destino, así que siempre vale 0.
4. **PC**: `PC + 4`, o el destino del branch/salto.

**Memoria.** Address space plano de 32 bits. Se implementa como un mapa de
páginas de 4 KiB que se crean recién al escribir, así se puede usar cualquier
dirección (incluido el stack al final del address space) sin reservar 4 GiB.
Lo nunca escrito se lee como 0.

**Fin del programa.** El simulador detecta:
- un salto a sí mismo (`done: j done`, `beq x2, x2, done`), la forma usual de
  terminar un programa bare-metal;
- `ecall` de salida (a7 = 10, 17 o 93);
- errores: instrucción inválida, PC desalineado o ecall no soportada, que se
  reportan explícitamente con la dirección.

Todos los registros empiezan en 0 (incluido `sp`). Si un programa necesita el
stack en otra dirección: `set sp 0x7FFFFFF0`.

## Instrucciones soportadas

Todo el RV32I base del anexo A:
`lb lh lw lbu lhu addi slli slti sltiu xori srli srai ori andi auipc sb sh sw
add sub sll slt sltu xor srl sra or and lui beq bne blt bge bltu bgeu jalr jal`,
además de `fence` (no hace nada), `ecall` y `ebreak` (detiene la ejecución).

## Extras implementados

- **Environment calls** estilo SPIM/RARS (número de servicio en `a7`):

  | a7 | Servicio |
  |---|---|
  | 1 | print_int (a0) |
  | 4 | print_string (a0 = dirección de un string terminado en `\0`) |
  | 5 | read_int → a0 |
  | 8 | read_string (a0 = buffer, a1 = tamaño) |
  | 10 | exit |
  | 11 | print_char (a0) |
  | 12 | read_char → a0 |
  | 17, 93 | exit con código a0 |
  | 34, 35, 36 | print_int en hex, binario y sin signo |

- **Carga de archivos ELF** (ELF32 RISC-V): se cargan los segmentos `PT_LOAD`
  y el PC empieza en el entry point. Se detecta automáticamente por la firma
  del archivo.
- **Desensamblador** con nombres ABI y pseudoinstrucciones (`li`, `mv`, `j`,
  `ret`, `nop`, `beqz`...). Se muestra en cada `step` y con `disasm`.
- **Breakpoints** y visualización de los cambios que hace cada instrucción.

## Pruebas

`tests/` tiene los programas de prueba y `tests/build_tests.sh` los compila a
binarios crudos en `tests/bin/` (requiere clang, llvm-mc y ld.lld; los
binarios ya vienen compilados).

| Programa | Qué prueba | Resultado esperado |
|---|---|---|
| `riscvtest.bin` | add, sub, and, or, slt, addi, lw, sw, beq, jal | `mem 100 103` → `19 00 00 00` (25) |
| `quicksort-O0.bin`, `quicksort-O2.bin` | quicksort recursivo sobre `{6,4,3,2,1,8,9}` en 0x1000 (C compilado, usa stack) | `memw 0x1000 7` → 1 2 3 4 6 8 9 |
| `symmetric-O0.bin`, `symmetric-O2.bin` | `is_symmetric` sobre `{6,4,4,1,2,2,1}` en 0x1000 | `a0 = 1` |
| `instrs.bin` | 40 pruebas, una por instrucción del anexo A (signo, little-endian, shifts, unsigned, x0...) | `a0 = 1` (si falla, `s0` = nº de la prueba) |
| `hello.bin` | ecalls: imprime texto, un entero y un char, y sale con código 7 | Imprime `Hola desde RISC-V! n = -42` |

Los programas de CPUlator se exportan como se indica en el anexo B
(Memory → Format: Raw → Save to file) y se cargan igual.
