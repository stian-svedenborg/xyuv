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
#include "../xyuv/src/to_string.h"
#include "../xyuv/src/pixel_packer.cpp"

using namespace xyuv;

std::vector<float> test_values = {
        0.0f,
        1.0f,
        0.5f,
        0.1f,
        0.01f,
        0.333333333333f
};

std::vector<std::pair<float, float>> test_ranges {
        {0.0f, 1.0f},
        { 0.062745098, 0.921568627 },
        { 0.062745098, 0.941176471 }
};

std::vector<std::pair<uint8_t , uint8_t >> test_bits {
        { 8, 0 },
        { 8, 2 },
        { 8, 8 },
        { 16, 0 },
        { 32, 0},
        { 8, 16},
        { 1, 0 },
        { 2, 0 },
        { 3, 0 },
        { 4, 0 },
        { 5, 0 },
        { 6, 0 },
        { 7, 0 },
        { 0, 16 },


};




static void encode_decode(float expected_value, const std::pair<float, float> & input_range, const std::pair<uint8_t , uint8_t> & bits ) {
    unorm_t unorm = to_unorm(expected_value, bits.first, bits.second, input_range);
    float observed_value = from_unorm(unorm, bits.first, bits.second, input_range);

    uint64_t max = ((0x1uLL << bits.first) - 1) << bits.second;
    ASSERT_NEAR(expected_value, observed_value, 1.1f / max );
}

TEST(Unorm, StudioRange) {
    for (auto & bits : test_bits) {
        SCOPED_TRACE( "UNORM" + to_string(bits.first) + "." + to_string(bits.second) );
        for ( auto & range : test_ranges) {
            SCOPED_TRACE( "Range [" + to_string(range.first) + ", " + to_string(range.second) + "]" );
            for (float value : test_values) {
                encode_decode(value, range, bits);
            }
        }
    }
}
