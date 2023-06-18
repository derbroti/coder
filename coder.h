// Coder
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

// Space conserving (LEB128) CPU data-state (memory: addr+value, register: idx+value) encoder
// (decodable from left-to-right and right-to-left)
//
// When used to store deltas, this coder can be used to track, replay and roll-back all data changes
// of 32 and 16 bit integer-registers from a set of up to 255 different registers,
// and 16 bit wide memory, all with unlimited size clock values.
//
// Encoding info:
// --------------
//
// 16 bit register entry:
// ----------------------
//
// |----------------------- 1 byte ----------------------------------|--------- n byte ----------|---- n byte -----|
// 1bit leb128_marker, 1bit (value & 1), 1bit two_reg=0, 5bit reg_idx, varint_leb128 (value >> 1), varint_leb128 clk
//
// NOTE leb128_marker:
// when reading entries right-to-left, we have to read until we encounter the leb128 marker of the
// following value. To indicate that value ends, we set the MSB of our entries leftmost byte.
//
// NOTE two_reg:
// The only indication that we store 16 or 32 bit wide registers is this bit.
// Depending on its value, we have to decode the following data differently:
//
//
// 32 bit register entry:
// ----------------------
//
// |------------------------------- 1 byte -----------------------------------|------------------ n byte -------------------|---- n byte -----|
// 1bit leb128_decode_marker, 1bit (value1 & 1), 1bit two_reg=1, 5bit reg_idx1, varint_leb128 (value2 << 15) | (value1 >> 1), varint_leb128 clk
//
// NOTE reg_idx2:
// Only idx1 is saved, idx2 has to be calculated:
//
// E.g.:
// reg_idx2 := if  reg_idx1 & (1<<4)  then  reg_idx1-1  else  reg_idx1 | (1<<4)
//
// Why this formula? In my case: idx2 is the high-word, idx1 is the low-word
// (the condition stems from the discrepancy of where PH, PC and SP, FP and OP are placed e.g.:
// (PH=18, PC=19) but (SH=31, SP=15)
//
// 16 bit memory:
// --------------
// |---- n byte ----|------ n byte -----|----- n byte ------|
// varint_leb128 clk, varint_leb128 addr, varint_leb128 value
//
//

#include <algorithm>
#include <deque>
#include <iostream>

using std::cout, std::endl;

#define TWO_REG_BIT (1 << 5)

class Coder {
public:
    enum dir_t { l2r, r2l };
    enum destr_t { non_destr, destr };

    Coder(): s_it(s.begin()), s_rit(s.rbegin()) {}

    void reset_iter(dir_t dir) {
        if (dir == l2r)
            s_it = s.begin();
        else
            s_rit = s.rbegin();
    }
    void reset_iter() {
        reset_iter(l2r);
        reset_iter(r2l);
    }

    bool print(std::deque<uint8_t> cmp = {}, bool silent = false) {
        if (! silent) {
            cout << std::hex;
            std::ranges::copy(s, std::ostream_iterator<int>(std::cout, " "));
            cout << std::dec << endl;
        }
        return cmp.empty() || s == cmp; // don't compare if cmp is not set
    }

    size_t get_size() {
        return s.size();
    }

    template <typename U>
    void _encode(U value) {
        do {
            s.push_back(value & trim);
            value >>= 7;
        } while (value != 0);
        s.back() |= mark;
    }

    void _encode_raw(uint8_t val) {
        s.push_back(val);
    }

    template <destr_t U, dir_t V, typename W>
    void _decode(W& value) {
        if (! is_valid<V>())
            return;

        if constexpr (V == l2r) {
            uint8_t cnt = 0;
            value       = 0;

            while (! (*s_it & mark)) {
                value |= ((W)(*s_it & trim) << cnt);
                cnt   += 7;
                if (! advance<U, V>(s_it))
                    break;
            }
            value |= ((W)(*s_it & trim)) << cnt;
            advance<U, V>(s_it);
        } else {
            value = *s_rit & trim;
            if (! advance<U, V>(s_rit))
                return;

            while (! (*s_rit & mark)) {
                value <<= 7;
                value |= *s_rit;
                if (! advance<U, V>(s_rit))
                    break;
            }
        }
    }

    template <destr_t U, dir_t V>
    void _decode_raw(uint8_t& val) {
        if (! is_valid<V>())
            return;

        if constexpr (V == Coder::l2r) {
            val = *s_it;
            advance<U, V>(s_it);
        } else {
            val = *s_rit;
            advance<U, V>(s_rit);
        }
    }

private:
    const uint8_t mark = 0x80;
    const uint8_t trim = 0x7F;

    std::deque<uint8_t>                   s;
    std::deque<uint8_t>::iterator         s_it;
    std::deque<uint8_t>::reverse_iterator s_rit;

    template <dir_t V>
    bool is_valid() {
        if constexpr (V == l2r)
            return s_it != s.end();
        else
            return s_rit != s.rend();
    }

    template <destr_t U, dir_t V, typename W>
    bool advance(W& it) {
        it++;
        if constexpr (U == destr) {
            if constexpr (V == l2r)
                s.pop_front();
            else
                s.pop_back();
        }
        return is_valid<V>();
    }
};

class MemCoder : public Coder {
public:
    template <typename T>
    void encode(T clk, uint32_t addr, uint16_t val) {
        _encode(clk);
        _encode(addr);
        _encode(val);
        ++elements;
    }

    template <Coder::destr_t U = Coder::non_destr, Coder::dir_t V = Coder::l2r, typename W>
    void decode(W& clk, uint32_t& addr, uint16_t& val) {
        if constexpr (U == Coder::destr) {
            if (get_size())
                --elements;
        }
        if constexpr (V == Coder::l2r) {
            _decode<U, V>(clk);
            _decode<U, V>(addr);
            _decode<U, V>(val);
        } else {
            _decode<U, V>(val);
            _decode<U, V>(addr);
            _decode<U, V>(clk);
        }
    }

    size_t num_elements() {
        return elements;
    }

private:
    using Coder::_encode, Coder::_decode, Coder::_encode_raw, Coder::_decode_raw;
    size_t elements = 0;
};

class RegCoder : public Coder {
public:
    size_t num_elements() {
        return elements;
    }

    template <typename T>
    void encode(T clk, uint8_t idx, uint16_t value) {
        reg_encode<false>(clk, idx, 0, value);
    }

    template <typename T>
    void encode(T clk, uint8_t idx1, uint16_t high_value, uint16_t low_value) {
        reg_encode<true>(clk, idx1, high_value, low_value);
    }

    template <Coder::destr_t U = Coder::non_destr, Coder::dir_t V = Coder::l2r, typename W>
    bool decode(W& clk, uint8_t& idx2, uint16_t& high_value, uint8_t& idx1, uint16_t& low_value) {
        uint8_t  raw;
        uint32_t tmp_value;
        bool     two_regs = false;

        if constexpr (U == Coder::destr) {
            if (get_size())
                --elements;
        }
        if constexpr (V == Coder::l2r) {
            _decode_raw<U, V>(raw);

            uint8_t value_LSB = (raw >> 6) & 1;
            idx1              = raw & 0x1F;
            two_regs          = (raw >> 5) & 1;

            _decode<U, V>(tmp_value);
            tmp_value = ((tmp_value) << 1) | value_LSB;
            _decode<U, V>(clk);
        } else {
            _decode<U, V>(clk);
            _decode<U, V>(tmp_value);
            _decode_raw<U, V>(raw);

            uint8_t value_LSB = (raw >> 6) & 1;
            idx1              = raw & 0x1F;
            tmp_value         = ((tmp_value) << 1) | value_LSB;
            two_regs          = (raw >> 5) & 1;
        }
        low_value  = tmp_value & 0xFFFF;
        high_value = tmp_value >> 16;

        idx2 = 0;
        if (two_regs)
            idx2 = decode_idx2(idx1);

        return two_regs;
    }

private:
    using Coder::_encode, Coder::_decode, Coder::_encode_raw, Coder::_decode_raw;
    size_t elements = 0;

    uint8_t decode_idx2(uint8_t idx1) {
        if (idx1 & (1 << 4))
            return idx1 - 1;
        else
            return idx1 | (1 << 4);
    }

    template <bool two, typename T>
    void reg_encode(T clk, uint8_t idx, uint16_t high_value, uint16_t low_value) {
        idx |= 0x80;                 //bit7 := 1 // leb128 decode marker
        idx |= (low_value & 1) << 6; //bit6 := value & 1
        if constexpr (two)           //
            idx |= TWO_REG_BIT;      //bit5 := 1
        else                         //
            idx &= ~TWO_REG_BIT;     //bit5 := 0
        _encode_raw(idx);

        _encode((high_value << 15) | (low_value >> 1));
        _encode(clk);
        ++elements;
    }
};
