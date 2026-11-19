// Decodificación de instrucciones RV32I.
// Se decodifica una sola vez a una estructura Instr, que usan tanto el
// ejecutor (cpu.cpp) como el desensamblador (disasm.cpp).
#pragma once

#include <cstdint>

enum class Op {
    INVALID,
    // Loads
    LB, LH, LW, LBU, LHU,
    // OP-IMM
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
    // U-type
    LUI, AUIPC,
    // Stores
    SB, SH, SW,
    // OP (R-type)
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // Branches
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    // Jumps
    JAL, JALR,
    // Sistema
    FENCE, ECALL, EBREAK,
};

struct Instr {
    Op op = Op::INVALID;
    uint32_t raw = 0;
    uint8_t rd = 0, rs1 = 0, rs2 = 0;
    int32_t imm = 0;  // ya con extensión de signo (o shamt para shifts)
};

Instr decode(uint32_t raw);
const char* op_name(Op op);
