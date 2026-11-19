#include "cpu.hpp"

#include <algorithm>
#include <string>

#include "util.hpp"

void Cpu::write_reg(uint8_t rd, uint32_t value) {
    if (rd == 0) return;  // x0 siempre vale 0
    last.reg_written = true;
    last.rd = rd;
    last.reg_old = x[rd];
    last.reg_new = value;
    x[rd] = value;
}

StepResult Cpu::step() {
    if (pc & 3) {
        error = "PC desalineado: " + hex32(pc);
        return StepResult::Error;
    }

    // 1. Fetch
    const uint32_t raw = mem.read32(pc);
    // 2. Decode
    const Instr in = decode(raw);

    last = Effects{};
    last.pc = pc;
    last.instr = in;

    if (in.op == Op::INVALID) {
        error = "instrucción inválida " + hex32(raw) + " en " + hex32(pc);
        return StepResult::Error;
    }

    const uint32_t a = x[in.rs1];
    const uint32_t b = x[in.rs2];
    const int32_t sa = int32_t(a), sb = int32_t(b);
    const uint32_t imm = uint32_t(in.imm);
    uint32_t next_pc = pc + 4;
    StepResult result = StepResult::Ok;

    auto store = [&](uint32_t addr, uint32_t value, int size) {
        if (size == 1) mem.write8(addr, uint8_t(value));
        else if (size == 2) mem.write16(addr, uint16_t(value));
        else mem.write32(addr, value);
        last.mem_written = true;
        last.mem_addr = addr;
        last.mem_size = size;
        last.mem_value = size == 4 ? value : value & ((1u << (8 * size)) - 1);
    };

    // 3. Execute / memory / writeback
    switch (in.op) {
    // ---- Loads ----
    case Op::LB:  write_reg(in.rd, uint32_t(int32_t(int8_t(mem.read8(a + imm))))); break;
    case Op::LH:  write_reg(in.rd, uint32_t(int32_t(int16_t(mem.read16(a + imm))))); break;
    case Op::LW:  write_reg(in.rd, mem.read32(a + imm)); break;
    case Op::LBU: write_reg(in.rd, mem.read8(a + imm)); break;
    case Op::LHU: write_reg(in.rd, mem.read16(a + imm)); break;

    // ---- Aritmética con inmediato ----
    case Op::ADDI:  write_reg(in.rd, a + imm); break;
    case Op::SLTI:  write_reg(in.rd, sa < in.imm ? 1 : 0); break;
    case Op::SLTIU: write_reg(in.rd, a < imm ? 1 : 0); break;
    case Op::XORI:  write_reg(in.rd, a ^ imm); break;
    case Op::ORI:   write_reg(in.rd, a | imm); break;
    case Op::ANDI:  write_reg(in.rd, a & imm); break;
    case Op::SLLI:  write_reg(in.rd, a << (imm & 31)); break;
    case Op::SRLI:  write_reg(in.rd, a >> (imm & 31)); break;
    case Op::SRAI:  write_reg(in.rd, uint32_t(sa >> (imm & 31))); break;

    case Op::LUI:   write_reg(in.rd, imm); break;
    case Op::AUIPC: write_reg(in.rd, pc + imm); break;

    // ---- Stores ----
    case Op::SB: store(a + imm, b, 1); break;
    case Op::SH: store(a + imm, b, 2); break;
    case Op::SW: store(a + imm, b, 4); break;

    // ---- Registro-registro ----
    case Op::ADD:  write_reg(in.rd, a + b); break;
    case Op::SUB:  write_reg(in.rd, a - b); break;
    case Op::SLL:  write_reg(in.rd, a << (b & 31)); break;
    case Op::SLT:  write_reg(in.rd, sa < sb ? 1 : 0); break;
    case Op::SLTU: write_reg(in.rd, a < b ? 1 : 0); break;
    case Op::XOR:  write_reg(in.rd, a ^ b); break;
    case Op::SRL:  write_reg(in.rd, a >> (b & 31)); break;
    case Op::SRA:  write_reg(in.rd, uint32_t(sa >> (b & 31))); break;
    case Op::OR:   write_reg(in.rd, a | b); break;
    case Op::AND:  write_reg(in.rd, a & b); break;

    // ---- Branches ----
    case Op::BEQ:  if (a == b)   next_pc = pc + imm; break;
    case Op::BNE:  if (a != b)   next_pc = pc + imm; break;
    case Op::BLT:  if (sa < sb)  next_pc = pc + imm; break;
    case Op::BGE:  if (sa >= sb) next_pc = pc + imm; break;
    case Op::BLTU: if (a < b)    next_pc = pc + imm; break;
    case Op::BGEU: if (a >= b)   next_pc = pc + imm; break;

    // ---- Saltos ----
    case Op::JAL:
        write_reg(in.rd, pc + 4);
        next_pc = pc + imm;
        break;
    case Op::JALR:
        // Se calcula el destino antes de escribir rd (por si rd == rs1).
        next_pc = (a + imm) & ~1u;
        write_reg(in.rd, pc + 4);
        break;

    case Op::FENCE: break;  // sin efecto en un simulador de un solo hilo
    case Op::ECALL: result = do_ecall(); break;
    case Op::EBREAK: result = StepResult::Breakpoint; break;
    case Op::INVALID: break;
    }

    // Un salto a sí mismo (j . / beq x0,x0,.) es la forma típica de "terminar"
    // un programa bare-metal; lo detectamos para no quedarnos colgados.
    if (result == StepResult::Ok && next_pc == pc) result = StepResult::Loop;

    pc = next_pc;
    instret++;
    return result;
}

// Environment calls estilo SPIM/RARS (número de servicio en a7 = x17).
StepResult Cpu::do_ecall() {
    const uint32_t service = x[17];
    const uint32_t a0 = x[10];
    switch (service) {
    case 1:  // print_int
        *out << int32_t(a0) << std::flush;
        break;
    case 4: {  // print_string (terminado en '\0')
        for (uint32_t p = a0;; p++) {
            char c = char(mem.read8(p));
            if (!c) break;
            *out << c;
        }
        *out << std::flush;
        break;
    }
    case 5: {  // read_int
        std::string line;
        std::getline(*in, line);
        write_reg(10, uint32_t(std::stol(line.empty() ? "0" : line, nullptr, 0)));
        break;
    }
    case 8: {  // read_string(a0 = buffer, a1 = tamaño máx.)
        std::string line;
        std::getline(*in, line);
        uint32_t max = x[11];
        if (max == 0) break;
        uint32_t n = std::min<uint32_t>(uint32_t(line.size()), max - 1);
        for (uint32_t i = 0; i < n; i++) mem.write8(a0 + i, uint8_t(line[i]));
        mem.write8(a0 + n, 0);
        break;
    }
    case 10:  // exit
        exit_code = 0;
        return StepResult::Exit;
    case 11:  // print_char
        *out << char(a0) << std::flush;
        break;
    case 12: {  // read_char
        int c = in->get();
        write_reg(10, c == EOF ? uint32_t(-1) : uint32_t(c));
        break;
    }
    case 17:  // exit2 (código en a0)
    case 93:  // exit (convención Linux)
        exit_code = int32_t(a0);
        return StepResult::Exit;
    case 34:  // print_int_hex
        *out << hex32(a0) << std::flush;
        break;
    case 35: {  // print_int_binary
        for (int i = 31; i >= 0; i--) *out << ((a0 >> i) & 1);
        *out << std::flush;
        break;
    }
    case 36:  // print_int_unsigned
        *out << a0 << std::flush;
        break;
    default:
        error = "ecall no soportada: a7 = " + std::to_string(service);
        return StepResult::Error;
    }
    return StepResult::Ok;
}
