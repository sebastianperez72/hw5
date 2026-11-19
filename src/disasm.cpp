#include "disasm.hpp"

#include <cstdarg>
#include <cstdio>

#include "util.hpp"

namespace {

std::string fmt(const char* f, ...) __attribute__((format(printf, 1, 2)));
std::string fmt(const char* f, ...) {
    char buf[128];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return buf;
}

std::string target(uint32_t pc, int32_t off) {
    char buf[16];
    std::snprintf(buf, sizeof buf, "0x%x", pc + uint32_t(off));
    return buf;
}

}  // namespace

std::string disassemble(const Instr& in, uint32_t pc) {
    const char* rd = abi_name(in.rd);
    const char* rs1 = abi_name(in.rs1);
    const char* rs2 = abi_name(in.rs2);
    const char* n = op_name(in.op);

    switch (in.op) {
    case Op::INVALID:
        return fmt(".word 0x%08x  # inválida", in.raw);

    case Op::LB: case Op::LH: case Op::LW: case Op::LBU: case Op::LHU:
        return fmt("%s %s, %d(%s)", n, rd, in.imm, rs1);

    case Op::SB: case Op::SH: case Op::SW:
        return fmt("%s %s, %d(%s)", n, rs2, in.imm, rs1);

    case Op::ADDI:
        if (in.rd == 0 && in.rs1 == 0 && in.imm == 0) return "nop";
        if (in.rs1 == 0) return fmt("li %s, %d", rd, in.imm);
        if (in.imm == 0) return fmt("mv %s, %s", rd, rs1);
        return fmt("addi %s, %s, %d", rd, rs1, in.imm);

    case Op::SLTI: case Op::SLTIU: case Op::XORI: case Op::ORI:
    case Op::ANDI: case Op::SLLI: case Op::SRLI: case Op::SRAI:
        return fmt("%s %s, %s, %d", n, rd, rs1, in.imm);

    case Op::LUI: case Op::AUIPC:
        return fmt("%s %s, 0x%x", n, rd, uint32_t(in.imm) >> 12);

    case Op::ADD: case Op::SUB: case Op::SLL: case Op::SLT: case Op::SLTU:
    case Op::XOR: case Op::SRL: case Op::SRA: case Op::OR: case Op::AND:
        return fmt("%s %s, %s, %s", n, rd, rs1, rs2);

    case Op::BEQ: case Op::BNE: case Op::BLT:
    case Op::BGE: case Op::BLTU: case Op::BGEU:
        if (in.rs2 == 0 && (in.op == Op::BEQ || in.op == Op::BNE))
            return fmt("%sz %s, %s", n, rs1, target(pc, in.imm).c_str());
        return fmt("%s %s, %s, %s", n, rs1, rs2, target(pc, in.imm).c_str());

    case Op::JAL:
        if (in.rd == 0) return "j " + target(pc, in.imm);
        if (in.rd == 1) return "jal " + target(pc, in.imm);
        return fmt("jal %s, %s", rd, target(pc, in.imm).c_str());

    case Op::JALR:
        if (in.rd == 0 && in.rs1 == 1 && in.imm == 0) return "ret";
        if (in.rd == 0 && in.imm == 0) return fmt("jr %s", rs1);
        return fmt("jalr %s, %d(%s)", rd, in.imm, rs1);

    case Op::FENCE: case Op::ECALL: case Op::EBREAK:
        return n;
    }
    return n;
}
