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

static xyuv::format create_continuation_block_format () {
    xyuv::format format;

    format.chroma_siting = Resources::config().get_chroma_siting("444");
    format.conversion_matrix = Resources::config().get_conversion_matrix("identity");

    format.image_w = 1;
    format.image_h = 1;

    format.size = 2;

    xyuv::plane plane;
    plane.size = 2;
    plane.base_offset = 0;
    plane.interleave_mode = xyuv::interleave_pattern::NO_INTERLEAVING;
    plane.block_stride = 16;
    plane.line_stride = 2;

    format.planes.push_back(plane);

    xyuv::channel_block & block = format.channel_blocks[xyuv::channel::Y];
    block.w = 1;
    block.h = 1;

    xyuv::sample s0;
    s0.integer_bits = 6;
    s0.fractional_bits = 0;
    s0.offset = 8;
    s0.plane = 0;
    s0.has_continuation = true;

    xyuv::sample s1;
    s1.integer_bits = 2;
    s1.fractional_bits = 0;
    s1.offset = 14;
    s1.plane = 0;
    s1.has_continuation = true;

    xyuv::sample s2;
    s2.integer_bits = 0;
    s2.fractional_bits = 4;
    s2.offset = 0;
    s2.plane = 0;
    s2.has_continuation = true;

    xyuv::sample s3;
    s3.integer_bits = 0;
    s3.fractional_bits = 4;
    s3.offset = 4;
    s3.plane = 0;
    s3.has_continuation = false;


    block.samples.push_back(s0);
    block.samples.push_back(s1);
    block.samples.push_back(s2);
    block.samples.push_back(s3);

    return format;
}


TEST(ContinuationBlocks, ContinuationBlocks) {
    xyuv::frame frame = xyuv::create_frame(create_continuation_block_format(), nullptr, 0);

    frame.data[0] = 0x55;
    frame.data[1] = 0x7f;

    yuv_image image = decode_frame(frame);

    xyuv::frame encoded_frame = encode_frame(image, frame.format);

    ASSERT_EQ(frame.data[0], encoded_frame.data[0]);
    ASSERT_EQ(frame.data[1], encoded_frame.data[1]);

}