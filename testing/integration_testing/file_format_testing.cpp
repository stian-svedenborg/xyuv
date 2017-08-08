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

#include "xyuv.h"
#include "xyuv/frame.h"
#include "xyuv/structures/format_template_old.h"
#include "../../xyuv/src/config_parser.h"
#include <gtest/gtest.h>

using namespace xyuv;

static format_template_old load_format(const std::string & filename) {
    std::string json;
    EXPECT_NO_THROW(json = read_json(filename));
    SCOPED_TRACE(filename);

    format_template_old result;
    EXPECT_NO_THROW(result = parse_format_template(json));

    return result;
}

static chroma_siting load_chroma_siting(const std::string & filename) {
    std::string json;
    SCOPED_TRACE(filename);
    EXPECT_NO_THROW(json = read_json(filename));

    chroma_siting result;
    EXPECT_NO_THROW(result = parse_chroma_siting(json));

    return result;
}

static conversion_matrix load_conversion_matrix(const std::string & filename) {
    std::string json;
    SCOPED_TRACE(filename);
    EXPECT_NO_THROW(json = read_json(filename));

    conversion_matrix result;
    EXPECT_NO_THROW(result = parse_conversion_matrix(json));

    return result;
}

void compare_headers(const ::format & expected, const ::format & observed) {
    #define COMPARE(field) ASSERT_EQ(expected.field, observed.field)
    COMPARE(image_w);
    COMPARE(image_h);
    COMPARE(size);
    COMPARE(origin);

    COMPARE(planes.size());
    for (std::size_t i = 0; i < expected.planes.size(); i++) {
        COMPARE(planes[i].base_offset);
        COMPARE(planes[i].size);
        COMPARE(planes[i].line_stride);
        COMPARE(planes[i].block_stride);
        COMPARE(planes[i].interleave_mode);
    }

    for ( uint32_t i = 0; i < expected.channel_blocks.size(); i++ ) {
        COMPARE(channel_blocks[i].w);
        COMPARE(channel_blocks[i].h);

        for (uint32_t j = 0; j < expected.channel_blocks[i].samples.size(); j++) {
            COMPARE(channel_blocks[i].samples[j].plane);
            COMPARE(channel_blocks[i].samples[j].integer_bits);
            COMPARE(channel_blocks[i].samples[j].fractional_bits);
            COMPARE(channel_blocks[i].samples[j].offset);
            COMPARE(channel_blocks[i].samples[j].has_continuation);
        }
    }
}

TEST(FileFormat, FileFormatSerialisation) {
    ::frame original_frame = create_frame(
            create_format(
                    50,
                    50,
                    load_format("formats/px_fmt/NV12"),
                    load_conversion_matrix("formats/rgb_conversion/bt601"),
                    load_chroma_siting("formats/chroma_siting/420")
            ),
            nullptr,
            0
    );

    std::ostringstream sout(std::ios::binary);
    write_frame(sout, original_frame);
    ASSERT_TRUE(sout.good());

    std::istringstream sin(sout.str(), std::ios::binary);
    ::frame reloaded_frame;
    read_frame( &reloaded_frame, sin);
    ASSERT_TRUE(sin.good());

    compare_headers(original_frame.format, reloaded_frame.format);
    ASSERT_EQ(memcmp(original_frame.data.get(), reloaded_frame.data.get(), original_frame.format.size), 0);
}

TEST(FileFormat, MultipleFramesInAFile) {
    constexpr uint32_t N = 4;
    std::array<::frame, N> frames;

    std::ostringstream sout(std::ios::binary);


    for (auto & frame : frames) {
        frame = create_frame(
                create_format(
                        50,
                        50,
                        load_format("formats/px_fmt/NV12"),
                        load_conversion_matrix("formats/rgb_conversion/bt601"),
                        load_chroma_siting("formats/chroma_siting/420")
                ),
                nullptr,
                0
        );

        write_frame(sout, frame);
        ASSERT_TRUE(sout.good());
    }

    std::istringstream sin(sout.str(), std::ios::binary);
    for (auto & original_frame : frames) {
        ::frame reloaded_frame;
        read_frame(&reloaded_frame, sin);
        ASSERT_TRUE(sin.good());

        compare_headers(original_frame.format, reloaded_frame.format);
        ASSERT_EQ(memcmp(original_frame.data.get(), reloaded_frame.data.get(), original_frame.format.size), 0);
    }
}

