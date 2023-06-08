// Coder
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

#include "../catch/catch_amalgamated.hpp"
#include "../coder.h"
#include "test.h"

TEST_CASE("RecCoder Tests", "") {
    uint64_t clk;
    uint16_t val1, val2;
    uint8_t  idx1, idx2;
    bool two_regs;
    RegCoder rc;

    SECTION("Encoding 1 - one-register") {
        rc.encode(0x1234567890abcdef, 0x5, 0x34ab);
        rc.encode(0x0, 0x0, 0x0);
        rc.encode(0x0, 0x1, 0x0);
        rc.encode(0x1, 0x0, 0x1);

        REQUIRE(rc.print({0xc5,0x55,0xb4,0x6f,0x1b,0x2f,0x5,0x9,0x4f,0x15,0x1a,0x92,
                          0x80,0x80,0x80,
                          0x81,0x80,0x80,
                          0xc0,0x80,0x81} , true));
        REQUIRE(rc.num_elements() == 4);

        SECTION("Destr. Decoding l2r") {
            rc.reset_iter();
            two_regs = rc.decode<Coder::destr>(clk, idx2, val2, idx1, val1);

            REQUIRE_FALSE(two_regs);
            REQUIRES(clk, 0x1234567890abcdef, idx2, 0x00, val2, 0x00, idx1, 0x05, val1, 0x34ab);
            REQUIRE(rc.num_elements() == 3);

            SECTION("Destr. Decoding l2r") {
                two_regs = rc.decode<Coder::destr>(clk, idx2, val2, idx1, val1);

                REQUIRE_FALSE(two_regs);
                REQUIRES(clk, 0x00, idx2, 0x00, val2, 0x00, idx1, 0x00, val1, 0x00);
                REQUIRE(rc.num_elements() == 2);

                SECTION("Destr. Decoding l2r") {
                    two_regs = rc.decode<Coder::destr>(clk, idx2, val2, idx1, val1);

                    REQUIRE_FALSE(two_regs);
                    REQUIRES(clk, 0x00, idx2, 0x00, val2, 0x00, idx1, 0x01, val1, 0x00);
                    REQUIRE(rc.num_elements() == 1);

                    SECTION("Destr. Decoding l2r") {
                        two_regs = rc.decode<Coder::destr>(clk, idx2, val2, idx1, val1);

                        REQUIRE_FALSE(two_regs);
                        REQUIRES(clk, 0x01, idx2, 0x00, val2, 0x00, idx1, 0x00, val1, 0x01);
                        REQUIRE(rc.num_elements() == 0);
                    }
                }
            }
        }
    }
    SECTION("Encoding 2 - two-register") {
        rc.encode(0x234bac4, 0x13, 0xabcd, 0x1234);
        rc.encode(0x12, 0xe, 0x25, 0x00);
        rc.encode(0x00, 0xf, 0x00, 0xf12);

        REQUIRE(rc.num_elements() == 3);
        REQUIRE(rc.print({0xb3,0x1a,0x12,0x1a,0x2f,0x85,0x44,0x75,0x52,0x91,
                          0xae,0x0,0x0,0xca,0x92,
                          0xaf,0x9,0x8f,0x80}, true));

        SECTION("Non-destr. Decoding r2l") {
            rc.reset_iter(Coder::r2l);
            two_regs = rc.decode<Coder::non_destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
            REQUIRE(two_regs);
            REQUIRE(rc.num_elements() == 3);
            REQUIRES(clk, 0x00, idx2, 0x1f, val2, 0x00, idx1, 0xf, val1, 0xf12);

            SECTION("Destr. Decoding l2r") {
                rc.reset_iter(Coder::l2r);
                two_regs = rc.decode<Coder::destr>(clk, idx2, val2, idx1, val1);
                REQUIRE(two_regs);
                REQUIRE(rc.num_elements() == 2);
                REQUIRES(clk, 0x234bac4, idx2, 0x12, val2, 0xabcd, idx1, 0x13, val1, 0x1234);

                SECTION("Destr. Decoding r2l") {
                    rc.reset_iter(Coder::r2l);
                    two_regs = rc.decode<Coder::destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
                    REQUIRE(two_regs);
                    REQUIRE(rc.num_elements() == 1);
                    REQUIRES(clk, 0x00, idx2, 0x1f, val2, 0x00, idx1, 0xf, val1, 0xf12);

                    SECTION("Destr. Decoding r2l") {
                        two_regs = rc.decode<Coder::destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
                        REQUIRE(two_regs);
                        REQUIRE(rc.num_elements() == 0);
                        REQUIRES(clk, 0x12, idx2, 0x1e, val2, 0x25, idx1, 0xe, val1, 0x00);
                    }
                }
            }
        }
    }
    SECTION("Encoding 3 - mixed one- and two-register") {
        rc.encode(0xf343, 0x11, 0x4456);
        rc.encode(0x12, 0x11, 0xabcd, 0xef01);
        rc.encode(0x123456789abcd, 0x1, 0x1111, 0x0);
        rc.encode(0x0, 21, 0xd00d);
        rc.encode(0x1, 17, 0xdd);
        rc.encode(0x2,  1, 0x1);

        REQUIRE(rc.print({0x91,0x2b,0xc4,0x43,0x66,0x83,
                          0xf1,0x0,0x6f,0x1b,0x2f,0x85,0x92,
                          0xa1,0x0,0x0,0x22,0xc4,0x4d,0x57,0x26,0x3c,0x56,0x68,0xc8,
                          0xd5,0x6,0x50,0x81,0x80,
                          0xd1,0xee,0x81,
                          0xc1,0x80,0x82}, true));
        REQUIRE(rc.num_elements() == 6);

        SECTION("Decoding mixed: l2r and r2l, destr and non-destr") {
            rc.reset_iter();
            two_regs = rc.decode<Coder::destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
            REQUIRE_FALSE(two_regs);
            REQUIRE(rc.num_elements() == 5);
            REQUIRES(clk, 0x2, idx1, 0x1, val1, 0x1);

            two_regs = rc.decode<Coder::non_destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
            REQUIRE_FALSE(two_regs);
            REQUIRE(rc.num_elements() == 5);
            REQUIRES(clk, 0x1, idx1, 17, val1, 0xdd);

            two_regs = rc.decode<Coder::destr, Coder::r2l>(clk, idx2, val2, idx1, val1);
            REQUIRE_FALSE(two_regs);
            REQUIRE(rc.num_elements() == 4);
            REQUIRES(clk, 0x0, idx1, 21, val1, 0xd00d);

            two_regs = rc.decode<Coder::destr, Coder::l2r>(clk, idx2, val2, idx1, val1);
            REQUIRE_FALSE(two_regs);
            REQUIRE(rc.num_elements() == 3);
            REQUIRES(clk, 0xf343, idx1, 0x11, val1, 0x4456);

            two_regs = rc.decode<Coder::destr, Coder::l2r>(clk, idx2, val2, idx1, val1);
            REQUIRE(two_regs);
            REQUIRE(rc.num_elements() == 2);
            REQUIRES(clk, 0x12, idx2, 0x10, val2, 0xabcd, idx1, 0x11, val1, 0xef01);
        }
    }
}

