/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Stian Valentin Svedenborg
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

static xyuv::format create_interleaved_format () {
    xyuv::format format;

    format.chroma_siting = Resources::get().config().get_chroma_siting("444");
    format.conversion_matrix = Resources::get().config().get_conversion_matrix("identity");

    format.image_w = 1;
    format.image_h = 5;

    format.size = 5;
    format.origin = xyuv::image_origin::UPPER_LEFT;

    xyuv::plane plane;
    plane.size = 5;
    plane.base_offset = 0;
    plane.interleave_mode = xyuv::interleave_pattern::INTERLEAVE_0_2_4__1_3_5;
    plane.block_stride = 8;
    plane.line_stride = 1;
    plane.block_order.mega_block_height = 1;
    plane.block_order.mega_block_width = 1;

    format.planes.push_back(plane);

    xyuv::channel_block & block = format.channel_blocks[xyuv::channel::Y];
    block.w = 1;
    block.h = 1;

    xyuv::sample s0;
    s0.integer_bits = 8;
    s0.fractional_bits = 0;
    s0.offset = 0;
    s0.plane = 0;
    s0.has_continuation = false;

    block.samples.push_back(s0);


    return format;
}


TEST(Interleave, Interleaving_1_3_5__0_2_4) {
    xyuv::format f1 = create_interleaved_format();
    f1.planes[0].interleave_mode = interleave_pattern::INTERLEAVE_1_3_5__0_2_4;
    xyuv::frame frame = xyuv::create_frame(f1, nullptr, 0);

    frame.data[0] = 1;
    frame.data[1] = 3;
    frame.data[2] = 0;
    frame.data[3] = 2;
    frame.data[4] = 4;

    yuv_image image = decode_frame(frame);

    // Check that lines are loaded correctly.
    format f2 = frame.format;
    f2.planes[0].interleave_mode = interleave_pattern::NO_INTERLEAVING;
    xyuv::frame linear_frame = encode_frame(image, f2);
    for (std::size_t i = 0; i < frame.format.size; i++) {
        ASSERT_EQ(static_cast<uint8_t>(i), linear_frame.data[i]);
    }

    // Check that lines are stored correcty.
    xyuv::frame encoded_frame = encode_frame(image, frame.format);
    for (std::size_t i = 0; i < frame.format.size; i++) {
        ASSERT_EQ(frame.data[i], encoded_frame.data[i]);
    }
}

TEST(Interleave, Interleaving_0_2_4__1_3_5) {
    xyuv::frame frame = xyuv::create_frame(create_interleaved_format(), nullptr, 0);

    frame.data[0] = 0;
    frame.data[1] = 2;
    frame.data[2] = 4;
    frame.data[3] = 1;
    frame.data[4] = 3;

    yuv_image image = decode_frame(frame);

    // Check that lines are loaded correctly.
    format f2 = frame.format;
    f2.planes[0].interleave_mode = interleave_pattern::NO_INTERLEAVING;
    xyuv::frame linear_frame = encode_frame(image, f2);
    for (std::size_t i = 0; i < frame.format.size; i++) {
        ASSERT_EQ(static_cast<uint8_t>(i), linear_frame.data[i]);
    }

    // Check that lines are stored correcty.
    xyuv::frame encoded_frame = encode_frame(image, frame.format);
    for (std::size_t i = 0; i < frame.format.size; i++) {
        ASSERT_EQ(frame.data[i], encoded_frame.data[i]);
    }
}