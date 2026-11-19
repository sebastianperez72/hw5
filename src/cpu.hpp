// Estado arquitectural de una CPU RV32I y ejecución de instrucciones.
#pragma once

#include <cstdint>
#include <iostream>
#include <string>

#include "decode.hpp"
#include "memory.hpp"

enum class StepResult {
    Ok,         // se ejecutó la instrucción normalmente
    Exit,       // el programa terminó vía ecall (exit)
    Loop,       // salto a sí mismo (p. ej. "done: j done"): fin del programa
    Breakpoint, // ebreak
    Error,      // instrucción inválida, PC desalineado, etc.
};

// Qué cambió al ejecutar la última instrucción (para mostrarlo al usuario).
struct Effects {
    uint32_t pc = 0;
    Instr instr;
    bool reg_written = false;
    uint8_t rd = 0;
    uint32_t reg_old = 0, reg_new = 0;
    bool mem_written = false;
    uint32_t mem_addr = 0;
    int mem_size = 0;
    uint32_t mem_value = 0;
};

class Cpu {
public:
    uint32_t pc = 0;
    uint32_t x[32] = {};
    Memory mem;

    uint64_t instret = 0;  // instrucciones ejecutadas
    int exit_code = 0;
    std::string error;
    Effects last;

    std::istream* in = &std::cin;
    std::ostream* out = &std::cout;

    void reset_regs() {
        pc = 0;
        for (auto& r : x) r = 0;
        instret = 0;
        exit_code = 0;
        error.clear();
    }

    StepResult step();

private:
    void write_reg(uint8_t rd, uint32_t value);
    StepResult do_ecall();
};
