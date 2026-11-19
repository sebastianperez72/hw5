#include "decode.hpp"

namespace {

// Extracción de inmediatos según el formato (ver RISC-V spec, cap. 2).
// El cast a int32_t antes del shift a la derecha hace la extensión de signo.
int32_t imm_i(uint32_t x) { return int32_t(x) >> 20; }

int32_t imm_s(uint32_t x) {
    return (int32_t(x & 0xFE000000) >> 20) | int32_t((x >> 7) & 0x1F);
}

int32_t imm_b(uint32_t x) {
    return (int32_t(x & 0x80000000) >> 19)  // imm[12]
           | int32_t((x & 0x80) << 4)       // imm[11]
           | int32_t((x >> 20) & 0x7E0)     // imm[10:5]
           | int32_t((x >> 7) & 0x1E);      // imm[4:1]
}

int32_t imm_u(uint32_t x) { return int32_t(x & 0xFFFFF000); }

int32_t imm_j(uint32_t x) {
    return (int32_t(x & 0x80000000) >> 11)  // imm[20]
           | int32_t(x & 0xFF000)           // imm[19:12]
           | int32_t((x >> 9) & 0x800)      // imm[11]
           | int32_t((x >> 20) & 0x7FE);    // imm[10:1]
}

}  // namespace

Instr decode(uint32_t x) {
    Instr in;
    in.raw = x;
    in.rd = (x >> 7) & 0x1F;
    in.rs1 = (x >> 15) & 0x1F;
    in.rs2 = (x >> 20) & 0x1F;
    const uint32_t opcode = x & 0x7F;
    const uint32_t f3 = (x >> 12) & 0x7;
    const uint32_t f7 = x >> 25;

    switch (opcode) {
    case 0x03:  // LOAD
        in.imm = imm_i(x);
        switch (f3) {
        case 0: in.op = Op::LB; break;
        case 1: in.op = Op::LH; break;
        case 2: in.op = Op::LW; break;
        case 4: in.op = Op::LBU; break;
        case 5: in.op = Op::LHU; break;
        }
        break;

    case 0x13:  // OP-IMM
        in.imm = imm_i(x);
        switch (f3) {
        case 0: in.op = Op::ADDI; break;
        case 2: in.op = Op::SLTI; break;
        case 3: in.op = Op::SLTIU; break;
        case 4: in.op = Op::XORI; break;
        case 6: in.op = Op::ORI; break;
        case 7: in.op = Op::ANDI; break;
        case 1:
            in.imm = in.rs2;  // shamt
            if (f7 == 0x00) in.op = Op::SLLI;
            break;
        case 5:
            in.imm = in.rs2;
            if (f7 == 0x00) in.op = Op::SRLI;
            else if (f7 == 0x20) in.op = Op::SRAI;
            break;
        }
        break;

    case 0x37: in.op = Op::LUI;   in.imm = imm_u(x); break;
    case 0x17: in.op = Op::AUIPC; in.imm = imm_u(x); break;

    case 0x23:  // STORE
        in.imm = imm_s(x);
        switch (f3) {
        case 0: in.op = Op::SB; break;
        case 1: in.op = Op::SH; break;
        case 2: in.op = Op::SW; break;
        }
        break;

    case 0x33:  // OP
        if (f7 == 0x00) {
            static const Op ops[8] = {Op::ADD, Op::SLL, Op::SLT, Op::SLTU,
                                      Op::XOR, Op::SRL, Op::OR,  Op::AND};
            in.op = ops[f3];
        } else if (f7 == 0x20) {
            if (f3 == 0) in.op = Op::SUB;
            else if (f3 == 5) in.op = Op::SRA;
        }
        break;

    case 0x63:  // BRANCH
        in.imm = imm_b(x);
        switch (f3) {
        case 0: in.op = Op::BEQ; break;
        case 1: in.op = Op::BNE; break;
        case 4: in.op = Op::BLT; break;
        case 5: in.op = Op::BGE; break;
        case 6: in.op = Op::BLTU; break;
        case 7: in.op = Op::BGEU; break;
        }
        break;

    case 0x6F: in.op = Op::JAL; in.imm = imm_j(x); break;

    case 0x67:
        if (f3 == 0) { in.op = Op::JALR; in.imm = imm_i(x); }
        break;

    case 0x0F: in.op = Op::FENCE; break;

    case 0x73:
        if (x == 0x00000073) in.op = Op::ECALL;
        else if (x == 0x00100073) in.op = Op::EBREAK;
        break;
    }
    return in;
}

const char* op_name(Op op) {
    switch (op) {
    case Op::LB: return "lb";       case Op::LH: return "lh";
    case Op::LW: return "lw";       case Op::LBU: return "lbu";
    case Op::LHU: return "lhu";     case Op::ADDI: return "addi";
    case Op::SLTI: return "slti";   case Op::SLTIU: return "sltiu";
    case Op::XORI: return "xori";   case Op::ORI: return "ori";
    case Op::ANDI: return "andi";   case Op::SLLI: return "slli";
    case Op::SRLI: return "srli";   case Op::SRAI: return "srai";
    case Op::LUI: return "lui";     case Op::AUIPC: return "auipc";
    case Op::SB: return "sb";       case Op::SH: return "sh";
    case Op::SW: return "sw";       case Op::ADD: return "add";
    case Op::SUB: return "sub";     case Op::SLL: return "sll";
    case Op::SLT: return "slt";     case Op::SLTU: return "sltu";
    case Op::XOR: return "xor";     case Op::SRL: return "srl";
    case Op::SRA: return "sra";     case Op::OR: return "or";
    case Op::AND: return "and";     case Op::BEQ: return "beq";
    case Op::BNE: return "bne";     case Op::BLT: return "blt";
    case Op::BGE: return "bge";     case Op::BLTU: return "bltu";
    case Op::BGEU: return "bgeu";   case Op::JAL: return "jal";
    case Op::JALR: return "jalr";   case Op::FENCE: return "fence";
    case Op::ECALL: return "ecall"; case Op::EBREAK: return "ebreak";
    case Op::INVALID: break;
    }
    return "???";
}
