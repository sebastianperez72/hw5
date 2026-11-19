// rv32sim: simulador interactivo del ISA RV32I.
//
// Uso: rv32sim [--run] [--max N] programa.bin|programa.elf
//
// Se carga el programa y se abre una consola donde se puede ejecutar paso a
// paso e inspeccionar el PC, los registros y la memoria. Escribe "help" para
// ver los comandos.

#include <cctype>
#include <cstdio>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "cpu.hpp"
#include "disasm.hpp"
#include "loader.hpp"
#include "util.hpp"

namespace {

constexpr uint64_t DEFAULT_RUN_LIMIT = 10'000'000;

struct Sim {
    Cpu cpu;
    std::string path;
    LoadInfo info;
    std::set<uint32_t> breakpoints;
    bool halted = false;
};

// ---------- parsing ----------

bool parse_u32(const std::string& s, uint32_t& out) {
    try {
        size_t pos;
        long long v = std::stoll(s, &pos, 0);  // acepta 0x.., decimal y negativos
        if (pos != s.size()) return false;
        out = uint32_t(v);
        return true;
    } catch (...) {
        return false;
    }
}

// Acepta x0..x31, nombres ABI (a0, sp, ...), fp y "pc" (devuelve 32).
int parse_reg(std::string s) {
    for (auto& c : s) c = char(std::tolower(c));
    if (s == "pc") return 32;
    if (s == "fp") return 8;
    if (s.size() >= 2 && s[0] == 'x') {
        uint32_t n;
        if (parse_u32(s.substr(1), n) && n < 32) return int(n);
    }
    for (int i = 0; i < 32; i++)
        if (s == abi_name(i)) return i;
    return -1;
}

std::string reg_label(int r) {
    return "x" + std::to_string(r) + " (" + abi_name(r) + ")";
}

// ---------- impresión ----------

void print_instr_at(const Cpu& cpu, uint32_t addr, bool mark_pc = true) {
    const uint32_t raw = cpu.mem.read32(addr);
    std::printf("%s %s  %08X  %s\n", (mark_pc && addr == cpu.pc) ? "=>" : "  ",
                hex32(addr).c_str(), raw, disassemble(decode(raw), addr).c_str());
}

void print_effects(const Effects& e) {
    std::printf("[%s] %08X  %s\n", hex32(e.pc).c_str(), e.instr.raw,
                disassemble(e.instr, e.pc).c_str());
    if (e.reg_written)
        std::printf("      %s: %s -> %s\n", reg_label(e.rd).c_str(),
                    hex32(e.reg_old).c_str(), hex32(e.reg_new).c_str());
    if (e.mem_written)
        std::printf("      mem[%s] <- 0x%0*X (%d byte%s)\n", hex32(e.mem_addr).c_str(),
                    e.mem_size * 2, e.mem_value, e.mem_size, e.mem_size > 1 ? "s" : "");
}

void print_all_regs(const Cpu& cpu) {
    std::printf("pc = %s\n", hex32(cpu.pc).c_str());
    for (int i = 0; i < 32; i++) {
        char label[16];
        std::snprintf(label, sizeof label, "x%-2d %-4s", i, abi_name(i));
        std::printf("%s = %s%s", label, hex32(cpu.x[i]).c_str(), (i % 4 == 3) ? "\n" : "   ");
    }
}

void print_mem(const Cpu& cpu, uint32_t start, uint32_t end) {
    if (end < start) std::swap(start, end);
    const uint64_t len = uint64_t(end) - start + 1;
    if (len > 4096) {
        std::printf("Rango demasiado grande (máx. 4096 bytes).\n");
        return;
    }
    if (len <= 16) {
        std::printf("Memoria (%s-%s):", hex32(start).c_str(), hex32(end).c_str());
        for (uint64_t a = start; a <= end; a++) std::printf(" %02X", cpu.mem.read8(uint32_t(a)));
        std::printf("\n");
        return;
    }
    // Volcado hexadecimal clásico: 16 bytes por línea + ASCII
    for (uint64_t line = start & ~0xFull; line <= end; line += 16) {
        std::printf("%s: ", hex32(uint32_t(line)).c_str());
        std::string ascii;
        for (uint64_t a = line; a < line + 16; a++) {
            if (a < start || a > end) {
                std::printf("   ");
                ascii += ' ';
            } else {
                uint8_t b = cpu.mem.read8(uint32_t(a));
                std::printf("%02X ", b);
                ascii += std::isprint(b) ? char(b) : '.';
            }
            if ((a & 15) == 7) std::printf(" ");
        }
        std::printf(" |%s|\n", ascii.c_str());
    }
}

void print_help() {
    std::printf(
        "Comandos:\n"
        "  step [n]            (s)  ejecuta n instrucciones (por defecto 1)\n"
        "  run [max]           (c)  ejecuta hasta terminar, breakpoint o error\n"
        "  pc                       muestra el PC y la instrucción actual\n"
        "  regs [r ...]        (r)  muestra todos los registros o solo los indicados\n"
        "                           (x5, t0, sp, a0, pc...)\n"
        "  set <reg> <valor>        cambia un registro (o el pc)\n"
        "  mem <inicio> [fin]  (m)  muestra bytes de memoria [inicio, fin]\n"
        "  memw <inicio> [n]        muestra n palabras de 32 bits (little-endian)\n"
        "  poke <addr> <valor>      escribe una palabra de 32 bits en memoria\n"
        "  disasm [addr] [n]   (d)  desensambla n instrucciones (por defecto, en el PC)\n"
        "  break <addr>        (b)  pone un breakpoint\n"
        "  delete <addr>            quita un breakpoint\n"
        "  breaks                   lista los breakpoints\n"
        "  info                     instrucciones ejecutadas y estado\n"
        "  reset                    recarga el programa y reinicia el estado\n"
        "  help                (h)  esta ayuda\n"
        "  exit                (q)  salir\n"
        "Los números aceptan decimal (100) o hexadecimal (0x64).\n");
}

// ---------- ejecución ----------

// Ejecuta una instrucción e informa si el programa terminó. Devuelve false si
// la ejecución debe detenerse.
bool exec_one(Sim& sim, bool verbose) {
    Cpu& cpu = sim.cpu;
    StepResult r = cpu.step();
    if (verbose) print_effects(cpu.last);
    switch (r) {
    case StepResult::Ok:
        return true;
    case StepResult::Exit:
        std::printf("\nEl programa terminó (ecall exit) con código %d.\n", cpu.exit_code);
        sim.halted = true;
        return false;
    case StepResult::Loop:
        std::printf("El programa terminó: bucle infinito en %s (salto a sí mismo).\n",
                    hex32(cpu.pc).c_str());
        sim.halted = true;
        return false;
    case StepResult::Breakpoint:
        std::printf("ebreak en %s.\n", hex32(cpu.last.pc).c_str());
        return false;
    case StepResult::Error:
        std::printf("Error de ejecución: %s\n", cpu.error.c_str());
        sim.halted = true;
        return false;
    }
    return false;
}

void cmd_step(Sim& sim, uint64_t n) {
    if (sim.halted) {
        std::printf("El programa ya terminó. Usa \"reset\" para reiniciarlo.\n");
        return;
    }
    const bool verbose = n <= 50;
    for (uint64_t i = 0; i < n; i++) {
        if (!exec_one(sim, verbose)) return;
        if (i + 1 < n && sim.breakpoints.count(sim.cpu.pc)) {
            std::printf("Breakpoint en %s.\n", hex32(sim.cpu.pc).c_str());
            return;
        }
    }
    if (!verbose) std::printf("%llu instrucciones ejecutadas. pc = %s\n",
                              (unsigned long long)n, hex32(sim.cpu.pc).c_str());
}

void cmd_run(Sim& sim, uint64_t limit) {
    if (sim.halted) {
        std::printf("El programa ya terminó. Usa \"reset\" para reiniciarlo.\n");
        return;
    }
    const uint64_t start = sim.cpu.instret;
    for (uint64_t i = 0; i < limit; i++) {
        if (!exec_one(sim, false)) {
            std::printf("%llu instrucciones ejecutadas.\n",
                        (unsigned long long)(sim.cpu.instret - start));
            return;
        }
        if (sim.breakpoints.count(sim.cpu.pc)) {
            std::printf("Breakpoint en %s.\n", hex32(sim.cpu.pc).c_str());
            print_instr_at(sim.cpu, sim.cpu.pc);
            return;
        }
    }
    std::printf("Se alcanzó el límite de %llu instrucciones (pc = %s).\n",
                (unsigned long long)limit, hex32(sim.cpu.pc).c_str());
}

bool do_load(Sim& sim) {
    sim.info = load_program(sim.path, sim.cpu);
    sim.halted = false;
    if (!sim.info.ok) {
        std::printf("Error al cargar: %s\n", sim.info.error.c_str());
        return false;
    }
    std::printf("\"%s\" cargado a memoria (%s, %u bytes en %s-%s, entry = %s).\n",
                sim.path.c_str(), sim.info.is_elf ? "ELF" : "binario crudo",
                sim.info.end - sim.info.start, hex32(sim.info.start).c_str(),
                hex32(sim.info.end ? sim.info.end - 1 : 0).c_str(),
                hex32(sim.info.entry).c_str());
    return true;
}

// Devuelve false cuando hay que salir.
bool handle(Sim& sim, const std::string& line) {
    std::istringstream ss(line);
    std::vector<std::string> args;
    for (std::string t; ss >> t;) args.push_back(t);
    if (args.empty()) return true;
    const std::string cmd = args[0];
    Cpu& cpu = sim.cpu;

    auto num = [&](size_t i, uint32_t def) -> uint32_t {
        if (i >= args.size()) return def;
        uint32_t v;
        if (!parse_u32(args[i], v)) throw std::runtime_error("número inválido: " + args[i]);
        return v;
    };

    if (cmd == "exit" || cmd == "quit" || cmd == "q") return false;

    if (cmd == "help" || cmd == "h" || cmd == "?") {
        print_help();
    } else if (cmd == "step" || cmd == "s") {
        cmd_step(sim, num(1, 1));
    } else if (cmd == "run" || cmd == "continue" || cmd == "c") {
        cmd_run(sim, args.size() > 1 ? num(1, 0) : DEFAULT_RUN_LIMIT);
    } else if (cmd == "pc") {
        std::printf("pc = %s\n", hex32(cpu.pc).c_str());
        print_instr_at(cpu, cpu.pc);
    } else if (cmd == "regs" || cmd == "r") {
        if (args.size() == 1) {
            print_all_regs(cpu);
        } else {
            for (size_t i = 1; i < args.size(); i++) {
                int r = parse_reg(args[i]);
                if (r < 0) std::printf("%s: registro desconocido\n", args[i].c_str());
                else if (r == 32) std::printf("pc = %s\n", hex32(cpu.pc).c_str());
                else
                    std::printf("%s = %s  (%d)\n", args[i].c_str(), hex32(cpu.x[r]).c_str(),
                                int32_t(cpu.x[r]));
            }
        }
    } else if (cmd == "set") {
        if (args.size() != 3) throw std::runtime_error("uso: set <reg> <valor>");
        int r = parse_reg(args[1]);
        uint32_t v = num(2, 0);
        if (r < 0) throw std::runtime_error("registro desconocido: " + args[1]);
        if (r == 32) cpu.pc = v;
        else if (r != 0) cpu.x[r] = v;
        sim.halted = false;
    } else if (cmd == "mem" || cmd == "m") {
        if (args.size() < 2) throw std::runtime_error("uso: mem <inicio> [fin]");
        uint32_t s = num(1, 0);
        print_mem(cpu, s, num(2, s + 15));
    } else if (cmd == "memw") {
        if (args.size() < 2) throw std::runtime_error("uso: memw <inicio> [n]");
        uint32_t s = num(1, 0), n = num(2, 8);
        for (uint32_t i = 0; i < n && i < 1024; i++) {
            uint32_t a = s + 4 * i, v = cpu.mem.read32(a);
            std::printf("%s: %s  (%d)\n", hex32(a).c_str(), hex32(v).c_str(), int32_t(v));
        }
    } else if (cmd == "poke") {
        if (args.size() != 3) throw std::runtime_error("uso: poke <addr> <valor>");
        cpu.mem.write32(num(1, 0), num(2, 0));
    } else if (cmd == "disasm" || cmd == "d") {
        uint32_t a = num(1, cpu.pc), n = num(2, 10);
        for (uint32_t i = 0; i < n && i < 1024; i++) print_instr_at(cpu, a + 4 * i);
    } else if (cmd == "break" || cmd == "b") {
        uint32_t a = num(1, cpu.pc);
        sim.breakpoints.insert(a);
        std::printf("Breakpoint en %s.\n", hex32(a).c_str());
    } else if (cmd == "delete") {
        uint32_t a = num(1, cpu.pc);
        std::printf(sim.breakpoints.erase(a) ? "Breakpoint eliminado.\n"
                                             : "No había breakpoint ahí.\n");
    } else if (cmd == "breaks") {
        if (sim.breakpoints.empty()) std::printf("No hay breakpoints.\n");
        for (uint32_t a : sim.breakpoints) print_instr_at(cpu, a);
    } else if (cmd == "info") {
        std::printf("Programa: %s\nInstrucciones ejecutadas: %llu\npc = %s\nEstado: %s\n",
                    sim.path.c_str(), (unsigned long long)cpu.instret,
                    hex32(cpu.pc).c_str(), sim.halted ? "terminado" : "en ejecución");
    } else if (cmd == "reset") {
        do_load(sim);
    } else {
        std::printf("Comando desconocido: %s (escribe \"help\")\n", cmd.c_str());
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    Sim sim;
    bool run_mode = false;
    uint64_t max = DEFAULT_RUN_LIMIT;

    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--run" || a == "-r") run_mode = true;
        else if ((a == "--max" || a == "-m") && i + 1 < argc) max = std::stoull(argv[++i], nullptr, 0);
        else if (a == "--help" || a == "-h") {
            std::printf("Uso: %s [--run] [--max N] programa.bin|programa.elf\n"
                        "  --run    ejecuta el programa completo y muestra los registros\n"
                        "  --max N  límite de instrucciones para run (por defecto %llu)\n",
                        argv[0], (unsigned long long)DEFAULT_RUN_LIMIT);
            return 0;
        } else sim.path = a;
    }
    if (sim.path.empty()) {
        std::fprintf(stderr, "Uso: %s [--run] [--max N] programa.bin|programa.elf\n", argv[0]);
        return 1;
    }
    if (!do_load(sim)) return 1;

    if (run_mode) {
        cmd_run(sim, max);
        print_all_regs(sim.cpu);
        return sim.cpu.exit_code;
    }

    std::printf("Escribe \"help\" para ver los comandos.\n");
    std::string line;
    while (true) {
        std::printf("> ");
        std::fflush(stdout);
        if (!std::getline(std::cin, line)) {
            std::printf("\n");
            break;
        }
        try {
            if (!handle(sim, line)) break;
        } catch (const std::exception& e) {
            std::printf("Error: %s\n", e.what());
        }
    }
    std::printf("See you next time...\nProgram exited with code %d.\n", sim.cpu.exit_code);
    return 0;
}
