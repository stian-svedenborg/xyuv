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

#include <xyuv.h>
#include "../xyuv/src/to_string.h"
#include "TestResources.h"

#include <gtest/gtest.h>
#include <xyuv/structures/format.h>
#include <xyuv/frame.h>
#include <xyuv/yuv_image.h>

using namespace xyuv;

class SinglePixelSingleChannelTest : public ::testing::Test {
private:
    // Initialized with everything but the samples
    xyuv::format fmt;

protected:

    const pixel_quantum TEST_VAL = 0.710692402f;

    xyuv::frame encode_pixel(const xyuv::format & fmt) {
        // Create single pixel single channel image.
        xyuv::yuv_image testimage = xyuv::create_yuv_image_444(1, 1, true, false, false, false);

        // Fill it with the value UNORM 8.8 0b 10110101 00111010 = 0xB53A = 0.710692402
        testimage.y_plane.set(0, 0, TEST_VAL );

        return xyuv::encode_frame(testimage, fmt);

    }

    // Overwrites the samples in the Y channel block of the default format and
    // returns it.
    const xyuv::format& get_format(const std::vector<xyuv::sample> &samples) {
        fmt.channel_blocks[xyuv::channel::Y].samples = samples;
        return fmt;
    }

    virtual void SetUp() override {
        fmt.chroma_siting = Resources::get().config().get_chroma_siting("444");
        fmt.conversion_matrix = Resources::get().config().get_conversion_matrix("identity");

        fmt.image_w = 1;
        fmt.image_h = 1;

        fmt.size = 2;

        xyuv::plane plane;
        plane.size = 2;
        plane.base_offset = 0;
        plane.interleave_mode = xyuv::interleave_pattern::NO_INTERLEAVING;
        plane.block_stride = 16;
        plane.line_stride = 2;

        fmt.planes.push_back(plane);

        xyuv::channel_block & block = fmt.channel_blocks[xyuv::channel::Y];
        block.w = 1;
        block.h = 1;
    }
};


TEST_F(SinglePixelSingleChannelTest, PlainUnorm_8_8) {
    std::vector<xyuv::sample> samples;
    xyuv::sample s;

    s.offset = 0;
    s.integer_bits = 8;
    s.fractional_bits = 8;
    s.plane = 0;
    samples.push_back(s);

    xyuv::format fmt = get_format(samples);

    // Encode the test image using fmt:
    xyuv::frame frame = encode_pixel(fmt);
    ASSERT_EQ(0x3a, frame.data[0]);
    ASSERT_EQ(0xb5, frame.data[1]);


    // Decode the same frame.
    xyuv::yuv_image decoded = xyuv::decode_frame(frame);
    ASSERT_EQ(TEST_VAL, decoded.y_plane[0][0]);
}

TEST_F(SinglePixelSingleChannelTest, ContinuationUnorm_8_8) {
    std::vector<xyuv::sample> samples;
    xyuv::sample s;

    s.offset = 8;
    s.integer_bits = 8;
    s.fractional_bits = 0;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 0;
    s.integer_bits = 0;
    s.fractional_bits = 8;
    s.plane = 0;
    s.has_continuation = false;
    samples.push_back(s);

    xyuv::format fmt = get_format(samples);

    // Encode the test image using fmt:
    xyuv::frame frame = encode_pixel(fmt);
    ASSERT_EQ(0x3a, frame.data[0]);
    ASSERT_EQ(0xb5, frame.data[1]);

    // Decode the same frame.
    xyuv::yuv_image decoded = xyuv::decode_frame(frame);
    ASSERT_EQ(TEST_VAL, decoded.y_plane[0][0]);
}

TEST_F(SinglePixelSingleChannelTest, ComplexContinuationUnorm_8_8) {
    std::vector<xyuv::sample> samples;
    xyuv::sample s;

    s.offset = 12;
    s.integer_bits = 4;
    s.fractional_bits = 0;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 8;
    s.integer_bits = 4;
    s.fractional_bits = 0;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 4;
    s.integer_bits = 0;
    s.fractional_bits = 4;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 0;
    s.integer_bits = 0;
    s.fractional_bits = 4;
    s.plane = 0;
    s.has_continuation = false;
    samples.push_back(s);


    xyuv::format fmt = get_format(samples);

    // Encode the test image using fmt:
    xyuv::frame frame = encode_pixel(fmt);
    ASSERT_EQ(0x3a, frame.data[0]);
    ASSERT_EQ(0xb5, frame.data[1]);

    // Decode the same frame.
    xyuv::yuv_image decoded = xyuv::decode_frame(frame);
    ASSERT_EQ(TEST_VAL, decoded.y_plane[0][0]);
}

TEST_F(SinglePixelSingleChannelTest, ShuffledContinuationUnorm_8_8) {
    std::vector<xyuv::sample> samples;
    xyuv::sample s;

    s.offset = 12;
    s.integer_bits = 4;
    s.fractional_bits = 0;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 4;
    s.integer_bits = 4;
    s.fractional_bits = 0;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 8;
    s.integer_bits = 0;
    s.fractional_bits = 4;
    s.plane = 0;
    s.has_continuation = true;
    samples.push_back(s);

    s.offset = 0;
    s.integer_bits = 0;
    s.fractional_bits = 4;
    s.plane = 0;
    s.has_continuation = false;
    samples.push_back(s);

    xyuv::format fmt = get_format(samples);

    // Encode the test image using fmt:
    xyuv::frame frame = encode_pixel(fmt);
    ASSERT_EQ(0x5a, frame.data[0]);
    ASSERT_EQ(0xb3, frame.data[1]);

    // Decode the same frame.
    xyuv::yuv_image decoded = xyuv::decode_frame(frame);
    ASSERT_EQ(TEST_VAL, decoded.y_plane[0][0]);

}





