#pragma once

#include <cstdint>
#include <string>

#include "decode.hpp"

// Devuelve la instrucción en ensamblador (con nombres ABI y pseudoinstrucciones
// comunes como li, mv, j, ret, nop). `pc` se usa para mostrar el destino
// absoluto de branches y saltos.
std::string disassemble(const Instr& in, uint32_t pc);
