#pragma once

#include <string>

#include "cpu.hpp"

struct LoadInfo {
    bool ok = false;
    std::string error;
    bool is_elf = false;
    uint32_t start = 0, end = 0;  // rango cargado [start, end)
    uint32_t entry = 0;
};

// Carga un programa en memoria. Si el archivo empieza con la firma ELF se
// interpreta como ELF32 RISC-V (se cargan los segmentos PT_LOAD y el PC
// empieza en el entry point). Si no, se trata como binario crudo y se copia
// tal cual a partir de la dirección 0x00000000.
LoadInfo load_program(const std::string& path, Cpu& cpu);
