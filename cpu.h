// CPU
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

#include <array>
#include <span>
#include "coder.h"

template <uint8_t mem_bits>
concept cpu_mem_constraint = 8 <= mem_bits && mem_bits <= 24;

template <bool track>
concept cpu_needs_tracking = track == true;

// BIG-endian
template <uint8_t mem_bits = 16, bool track = false>
requires cpu_mem_constraint<mem_bits>
class Cpu_t {
public:
    uint8_t id;
    uint32_t pc; // ProgramCounter
    uint64_t clk; // Indicates the next to-be-executed instruction

    std::span<const uint16_t> mem_view = memory;

    Cpu_t(uint8_t id): id(id), pc(0), clk(0) {}

    void step() {
        execute(fetch());
        ++pc;
        ++clk;
    }

    template <bool tracking = track>
    requires cpu_needs_tracking<tracking>
    void sync(uint64_t targetClk) {
        while (clk < targetClk) {
            step();
        }
        mc.reset_iter(Coder::r2l);
        while (mc.num_elements() > 0 && clk > targetClk) {
            step_back();
        }
        lastMemClk = targetClk;
    }
    void step_back() {
        uint64_t clkDelta;
        uint32_t addr;
        uint16_t val;
        mc.decode<Coder::destr, Coder::r2l>(clkDelta, addr, val);
        clk -= clkDelta;
        memory[addr] -= val;
    }

    void execute(uint32_t inst) {
        // DEMO
        switch (inst >> 28) {
            case 0x00:
                auto dest = (inst & 0xFFFF) & MEMORY;
                write_memory(dest, (inst >> 16) & 0xFFF);
                break;
        }
        ///////
    }

    void set_inst(uint32_t addr, uint32_t value) {
        write_memory(addr,   value >> 16);
        write_memory(addr+1, value & 0xFFFF);
    }
    uint16_t get_memory(uint32_t addr) {
        return memory[addr & MEMORY];
    }

private:
    static constexpr uint32_t MEMORY = (1 << (mem_bits + 1)) - 1;
    static const uint8_t REGISTERS = 32;
    std::array<uint16_t, MEMORY> memory{0x0000};
    std::array<uint16_t, REGISTERS> registers {0x0000};

    MemCoder mc;
    uint64_t lastMemClk = 0;
    uint64_t lastRegClk = 0;

    uint32_t fetch() {
        return (memory[pc] << 16) | memory[pc+1];
    }

    void write_memory(uint32_t addr, uint16_t value) {
        if constexpr (track) {
            auto clkDelta = clk - lastMemClk;
            auto valDelta = value - memory[addr];

            mc.encode(clkDelta, addr, valDelta);

            lastMemClk = clk;
        }
        memory[addr & MEMORY] = value;
    }
};
