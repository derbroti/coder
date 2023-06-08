// CPU
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

#include "../catch/catch_amalgamated.hpp"
#include "test.h"
#include "../cpu.h"
#include <iostream>
#include <algorithm>
#include <ranges>

TEST_CASE("CPU Tests", "") {
    auto is_zero = [](const auto& v) { return v == 0; };
    std::cout << std::hex;

    Cpu_t<8, true> cpu(0);
    cpu.set_inst(0x00, 0x0123ABCD);
    cpu.clk = 1;

    SECTION("Write Memory") {
        cpu.step();
        REQUIRE(cpu.get_memory(0xABCD) == 0x123);
        REQUIRE(cpu.pc == 1);
        REQUIRE(cpu.clk == 2);

        SECTION("Sync to clk == 1") {
            cpu.sync(1);
            REQUIRE(std::all_of(cpu.mem_view.begin()+2, cpu.mem_view.end(), is_zero));

            SECTION("Sync to clk == 0") {
                cpu.sync(0);
                REQUIRE(std::ranges::all_of(cpu.mem_view, is_zero));
            }
        }
    }
}
