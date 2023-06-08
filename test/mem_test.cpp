// Coder
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

#include "../catch/catch_amalgamated.hpp"
#include "../coder.h"
#include "test.h"

TEST_CASE("MemCoder Tests", "") {
    uint64_t clk;
    uint32_t addr;
    uint16_t val;
    MemCoder mc;

    SECTION("Encoding 1") {
        mc.encode(0x1000, 0xFFAA33, 0xf100);
        mc.encode(0x2345, 0x29AA30, 0x112f);

        REQUIRE(mc.print({0x0,0xa0,0x33,0x54,0x7e,0x87,0x0,0x62,0x83,
                          0x45,0xc6,0x30,0x54,0x26,0x81,0x2f,0xa2}, true));
        REQUIRE(mc.num_elements() == 2);

        SECTION("Destr. Decoding l2r") {
            mc.reset_iter();
            mc.decode<Coder::destr>(clk, addr, val);
            REQUIRES(clk, 0x1000, addr, 0xFFAA33, val, 0xf100);

            REQUIRE(mc.num_elements() == 1);
            REQUIRE(mc.print({0x45,0xc6,0x30,0x54,0x26,0x81,0x2f,0xa2}, true));

            SECTION("Destr. Decoding l2r") {
                mc.decode<Coder::destr>(clk, addr, val);

                REQUIRE(mc.num_elements() == 0);
                REQUIRE(mc.get_size() == 0);

                SECTION("Decode Empty") {
                    mc.decode<Coder::destr>(clk, addr, val);

                    REQUIRE(mc.num_elements() == 0);
                    REQUIRE(mc.get_size() == 0);
                }
            }
        }
    }
    SECTION("Encoding 2") {
        mc.encode(0x0, 0x0, 0x12);
        mc.encode(0x424976272733345, 0x80, 0x0);

        REQUIRE(mc.print({0x80,0x80,0x92,
                          0x45,0x66,0x4c,0x13,0x27,0x6c,0x25,0x12,0x84,0x0,0x81,0x80}, true));
        REQUIRE(mc.num_elements() == 2);

        SECTION("Non-Destr. Decoding l2r") {
            mc.reset_iter();
            mc.decode(clk, addr, val);
            REQUIRES(clk, 0x00, addr, 0x00, val, 0x12);
            REQUIRE(mc.num_elements() == 2);

            mc.decode(clk, addr, val);
            REQUIRES(clk, 0x424976272733345, addr, 0x80, val, 0x00);
            REQUIRE(mc.num_elements() == 2);

            SECTION("Non-Destr. Decoding l2r - iterator limit") {
                mc.decode(clk, addr, val);
            }
        }
        SECTION("Non-Destr. Decoding r2l") {
            mc.reset_iter();
            mc.decode<Coder::non_destr, Coder::r2l>(clk, addr, val);
            REQUIRES(clk, 0x424976272733345,addr, 0x80, val, 0x00);
            REQUIRE(mc.num_elements() == 2);

            mc.decode<Coder::non_destr, Coder::r2l>(clk, addr, val);
            REQUIRES(clk, 0x00, addr, 0x00, val, 0x12);
            REQUIRE(mc.num_elements() == 2);

            SECTION("Non-Destr. Decoding r2l - iterator limit") {
                mc.decode(clk, addr, val);
            }
        }
    }
    SECTION("Ierator not reset") {
        mc.decode(clk, addr, val);
    }
}

