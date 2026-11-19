// Memoria plana byte-addressable de 32 bits, little-endian.
// Se implementa como memoria dispersa por páginas de 4 KiB: solo se reserva
// lo que el programa realmente toca, así el address space completo (4 GiB)
// "existe" sin gastar RAM. Las direcciones nunca escritas leen 0.
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>

class Memory {
public:
    static constexpr uint32_t PAGE_BITS = 12;
    static constexpr uint32_t PAGE_SIZE = 1u << PAGE_BITS;

    uint8_t read8(uint32_t addr) const {
        auto it = pages_.find(addr >> PAGE_BITS);
        if (it == pages_.end()) return 0;
        return (*it->second)[addr & (PAGE_SIZE - 1)];
    }

    void write8(uint32_t addr, uint8_t value) {
        auto& page = pages_[addr >> PAGE_BITS];
        if (!page) {
            page = std::make_unique<Page>();
            page->fill(0);
        }
        (*page)[addr & (PAGE_SIZE - 1)] = value;
    }

    // Little-endian: el byte menos significativo va en la dirección más baja.
    uint16_t read16(uint32_t addr) const {
        return uint16_t(read8(addr)) | uint16_t(read8(addr + 1)) << 8;
    }
    uint32_t read32(uint32_t addr) const {
        return uint32_t(read8(addr)) | uint32_t(read8(addr + 1)) << 8 |
               uint32_t(read8(addr + 2)) << 16 | uint32_t(read8(addr + 3)) << 24;
    }
    void write16(uint32_t addr, uint16_t v) {
        write8(addr, uint8_t(v));
        write8(addr + 1, uint8_t(v >> 8));
    }
    void write32(uint32_t addr, uint32_t v) {
        for (int i = 0; i < 4; i++) write8(addr + i, uint8_t(v >> (8 * i)));
    }

    void clear() { pages_.clear(); }

private:
    using Page = std::array<uint8_t, PAGE_SIZE>;
    std::unordered_map<uint32_t, std::unique_ptr<Page>> pages_;
};
