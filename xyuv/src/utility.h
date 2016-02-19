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

#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <xyuv/structures/color.h>

namespace xyuv {

std::vector<std::string> list_files_in_folder(const std::string &dir_path);

static inline pixel_quantum clamp( pixel_quantum min_, pixel_quantum max_, pixel_quantum val)
{
    using std::min;
    using std::max;
    return min(max_, max(min_, val));
}

uint32_t gcd(uint32_t u, uint32_t v);
uint32_t lcm(uint32_t a, uint32_t b);

    uint32_t next_multiple(uint32_t base, uint32_t multiplier);


//! \brief Buffer is seen as a continuous stream of bits from lsb of LSB to msb of MSB.
//! Offset is in bits from least significant bit of buffer to least significant bit of value

inline void set_bit(uint8_t *buffer, uint64_t offset, bool val) {
    uint8_t &byte = buffer[offset / 8];
    uint8_t mask = static_cast<uint8_t>(0x1 << (offset % 8));
    byte = static_cast<uint8_t>((val ? mask : 0) | (byte & ~mask));
}

//! \brief Buffer is seen as a continuous stream of bits from lsb of LSB to msb of MSB.
//! Offset is in bits from least significant bit of buffer to least significant bit of value
inline bool get_bit(const uint8_t *buffer, uint64_t offset) {
    uint8_t mask = static_cast<uint8_t>(0x1 << (offset % 8));
    return (buffer[offset / 8] & mask) != 0;
}

} // namespace xyuv