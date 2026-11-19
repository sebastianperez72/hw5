#include "loader.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

namespace {

uint16_t le16(const std::vector<uint8_t>& d, size_t o) {
    return uint16_t(d[o] | d[o + 1] << 8);
}
uint32_t le32(const std::vector<uint8_t>& d, size_t o) {
    return uint32_t(d[o]) | uint32_t(d[o + 1]) << 8 | uint32_t(d[o + 2]) << 16 |
           uint32_t(d[o + 3]) << 24;
}

LoadInfo load_elf(const std::vector<uint8_t>& d, Cpu& cpu) {
    LoadInfo info;
    info.is_elf = true;
    if (d.size() < 52 || d[4] != 1 /*ELFCLASS32*/ || d[5] != 1 /*little-endian*/) {
        info.error = "solo se soportan ELF de 32 bits little-endian";
        return info;
    }
    if (le16(d, 18) != 0xF3) {
        info.error = "el ELF no es de RISC-V (e_machine != 0xF3)";
        return info;
    }
    const uint32_t entry = le32(d, 24);
    const uint32_t phoff = le32(d, 28);
    const uint16_t phentsize = le16(d, 42);
    const uint16_t phnum = le16(d, 44);

    bool any = false;
    info.start = UINT32_MAX;
    for (uint16_t i = 0; i < phnum; i++) {
        const size_t ph = size_t(phoff) + size_t(i) * phentsize;
        if (ph + 32 > d.size()) break;
        if (le32(d, ph) != 1 /*PT_LOAD*/) continue;
        const uint32_t offset = le32(d, ph + 4);
        const uint32_t vaddr = le32(d, ph + 8);
        const uint32_t filesz = le32(d, ph + 16);
        const uint32_t memsz = le32(d, ph + 20);
        if (size_t(offset) + filesz > d.size()) {
            info.error = "segmento PT_LOAD fuera del archivo";
            return info;
        }
        for (uint32_t j = 0; j < memsz; j++)
            cpu.mem.write8(vaddr + j, j < filesz ? d[offset + j] : 0);
        info.start = std::min(info.start, vaddr);
        info.end = std::max(info.end, vaddr + memsz);
        any = true;
    }
    if (!any) {
        info.error = "el ELF no tiene segmentos cargables";
        return info;
    }
    info.entry = entry;
    cpu.pc = entry;
    info.ok = true;
    return info;
}

}  // namespace

LoadInfo load_program(const std::string& path, Cpu& cpu) {
    LoadInfo info;
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        info.error = "no se pudo abrir \"" + path + "\"";
        return info;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());

    cpu.mem.clear();
    cpu.reset_regs();

    if (data.size() >= 4 && data[0] == 0x7F && data[1] == 'E' && data[2] == 'L' &&
        data[3] == 'F')
        return load_elf(data, cpu);

    // Binario crudo -> dirección 0x00000000
    for (size_t i = 0; i < data.size(); i++) cpu.mem.write8(uint32_t(i), data[i]);
    info.ok = true;
    info.start = 0;
    info.end = uint32_t(data.size());
    info.entry = 0;
    cpu.pc = 0;
    return info;
}
