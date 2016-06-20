/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Stian Valentin Svedenborg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <gtest/gtest.h>

// This file tests the low-level get and set functions in pixelpacker.cpp

// This must be synced with pixelpacker.cpp
using unorm_t = uint64_t;

// These are the funcitons to test.
namespace xyuv {
    extern void set_bit(uint8_t *buffer, uint64_t offset, bool val);
    extern bool get_bit(const uint8_t *buffer, uint64_t offset);


    extern void write_bits(uint8_t *buffer, uint64_t offset, uint8_t bits, unorm_t &value);
    extern unorm_t read_bits(unorm_t &unorm, const uint8_t *buffer, uint64_t offset, uint8_t bits);
};

TEST(BitPacking, SetBit) {
    uint8_t byte = 0x00;

    // Set a single byte.
    xyuv::set_bit(&byte, 0, true);
    ASSERT_EQ(0x01, byte);

    // Unset a single byte.
    xyuv::set_bit(&byte, 0, false);
    ASSERT_EQ(0x00, byte);

    // Set a single byte.
    xyuv::set_bit(&byte, 1, true);
    ASSERT_EQ(0x02, byte);

    // Set a single byte.
    xyuv::set_bit(&byte, 0, true);
    ASSERT_EQ(0x03, byte);

}


// These are very simple, handwritten tests.
TEST(BitPacking, WriteSingleByte) {

    const uint8_t DEFAULT_VALUE = 0x5a;
    const unorm_t test_value    = 0xdc; // 0b11011100
    unorm_t  temp_val;
    // Byte filled
    uint8_t byte = DEFAULT_VALUE;

    // Write All 8 bits to the byte
    byte = DEFAULT_VALUE;

    temp_val = test_value;
    xyuv::write_bits(&byte, 0, 8, temp_val);
    ASSERT_EQ(0xdc, byte);

    // Write low 4 bits to low nibble
    byte = DEFAULT_VALUE;
    temp_val = test_value;
    xyuv::write_bits(&byte, 0, 4, temp_val);
    ASSERT_EQ(0x5c, byte);

    // Write low 4 bits to high nibble
    byte = DEFAULT_VALUE;
    temp_val = test_value;
    xyuv::write_bits(&byte, 4, 4, temp_val);
    ASSERT_EQ(0xca, byte);

}

// These are very simple, handwritten tests.
TEST(BitPacking, WriteMultiByte) {

    const uint8_t DEFAULT_VALUE = 0x5a;   // 0b 0101101
    const unorm_t TEST_VALUE    = 0xdcdc; // 0b 11011100 11011100
    unorm_t test_value;
    uint8_t bytes[2];

    // Write All 8 bits to the byte
    bytes[0] = DEFAULT_VALUE;
    bytes[1] = DEFAULT_VALUE;
    test_value = TEST_VALUE;
    xyuv::write_bits(bytes, 0, 8, test_value);
    ASSERT_EQ(0xdc, bytes[0]);
    ASSERT_EQ(DEFAULT_VALUE, bytes[1]);

    // Write low 4 bits to low nibble
    bytes[0] = DEFAULT_VALUE;
    bytes[1] = DEFAULT_VALUE;
    test_value = TEST_VALUE;
    xyuv::write_bits(bytes, 0, 4, test_value);
    ASSERT_EQ(0x5c, bytes[0]);
    ASSERT_EQ(DEFAULT_VALUE, bytes[1]);

    // Write low 4 bits to high nibble
    bytes[0] = DEFAULT_VALUE;
    bytes[1] = DEFAULT_VALUE;
    test_value = TEST_VALUE;
    xyuv::write_bits(bytes, 4, 4, test_value);
    ASSERT_EQ(0xca, bytes[0]);
    ASSERT_EQ(DEFAULT_VALUE, bytes[1]);

    // Write all 16 bits to the bytes
    bytes[0] = DEFAULT_VALUE;
    bytes[1] = DEFAULT_VALUE;
    test_value = TEST_VALUE;
    xyuv::write_bits(bytes, 0, 16, test_value);
    ASSERT_EQ(0xdc, bytes[0]);
    ASSERT_EQ(0xdc, bytes[1]);

    // Write low 5 bits onto_border
    bytes[0] = DEFAULT_VALUE;
    bytes[1] = DEFAULT_VALUE;
    test_value = TEST_VALUE;
    xyuv::write_bits(bytes, 6, 5, test_value);
    // Before write 0b 01011010 01011010
    // After write  0b 01011111 00011010
    ASSERT_EQ(0x1a, bytes[0]);
    ASSERT_EQ(0x5f, bytes[1]);
}

TEST(BitPacking, GetBit) {
    uint8_t byte = 0x5a; // 0x01011010

    // Set a single byte.
    ASSERT_FALSE(xyuv::get_bit(&byte, 0));
    ASSERT_TRUE (xyuv::get_bit(&byte, 1));
    ASSERT_FALSE(xyuv::get_bit(&byte, 2));
    ASSERT_TRUE (xyuv::get_bit(&byte, 3));
    ASSERT_TRUE (xyuv::get_bit(&byte, 4));
    ASSERT_FALSE(xyuv::get_bit(&byte, 5));
    ASSERT_TRUE (xyuv::get_bit(&byte, 6));
    ASSERT_FALSE(xyuv::get_bit(&byte, 7));
}

TEST(BitPacking, GetBitMultibyte) {        // <-- Increasing Byte addr  <--
    const uint8_t bytes[2] = {0x5a, 0xdc}; // 0b 11011100 01011010

    // Set a single byte.
    ASSERT_FALSE(xyuv::get_bit(bytes, 0));
    ASSERT_TRUE (xyuv::get_bit(bytes, 1));
    ASSERT_FALSE(xyuv::get_bit(bytes, 2));
    ASSERT_TRUE (xyuv::get_bit(bytes, 3));
    ASSERT_TRUE (xyuv::get_bit(bytes, 4));
    ASSERT_FALSE(xyuv::get_bit(bytes, 5));
    ASSERT_TRUE (xyuv::get_bit(bytes, 6));
    ASSERT_FALSE(xyuv::get_bit(bytes, 7));

    ASSERT_FALSE(xyuv::get_bit(bytes, 8 ));
    ASSERT_FALSE(xyuv::get_bit(bytes, 9 ));
    ASSERT_TRUE (xyuv::get_bit(bytes, 10));
    ASSERT_TRUE (xyuv::get_bit(bytes, 11));
    ASSERT_TRUE (xyuv::get_bit(bytes, 12));
    ASSERT_FALSE(xyuv::get_bit(bytes, 13));
    ASSERT_TRUE (xyuv::get_bit(bytes, 14));
    ASSERT_TRUE (xyuv::get_bit(bytes, 15));
}

TEST(BitPacking, ReadSinglebyte)
{
    const uint8_t byte = 0xdc; // 0b11011100
    unorm_t result;

    // Read all 8 bits from the byte
    result = 0;
    xyuv::read_bits(result, &byte, 0, 8);
    ASSERT_EQ(0xdc, result);

    // Read low 4 bits from low nibble
    result = 0;
    xyuv::read_bits(result, &byte, 0, 4);
    ASSERT_EQ(0x0c, result);

    // Read low 4 bits from high nibble
    result = 0;
    xyuv::read_bits(result, &byte, 4, 4);
    ASSERT_EQ(0x0d, result);

}


TEST(BitPacking, ReadMultibyte)
{                                               // <-- Increasing Byte addr  <--
    const uint8_t bytes[2] = { 0xdc, 0x5a };    // 0b 01011010 11011100
    unorm_t result;

    // Read all 8 bits from the low byte
    result = 0;
    xyuv::read_bits(result, bytes, 0, 8);
    ASSERT_EQ(0xdc, result);

    // Read low 4 bits from low nibble low byte
    result = 0;
    xyuv::read_bits(result, bytes, 0, 4);
    ASSERT_EQ(0x0c, result);

    // Read low 4 bits from high nibble low byte
    result = 0;
    xyuv::read_bits(result, bytes, 4, 4);
    ASSERT_EQ(0x0d, result);

    // Read all 8 bits from the high byte
    result = 0;
    xyuv::read_bits(result, bytes, 8, 8);
    ASSERT_EQ(0x5a, result);

    // Read low 4 bits from low nibble
    result = 0;
    xyuv::read_bits(result, bytes, 6, 5);
    ASSERT_EQ(0x0b, result);

    // Read low 12 bits crossing the byte-boundary
    result = 0;
    xyuv::read_bits(result, bytes, 3, 12);
    ASSERT_EQ(0xb5b, result);
}