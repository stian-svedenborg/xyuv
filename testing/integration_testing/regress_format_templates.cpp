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

#include <xyuv/surface.h>
#include <xyuv/yuv_image.h>
#include <xyuv/structures/format_template.h>
#include <xyuv/frame.h>
#include <xyuv.h>
#include "../../xyuv/src/config_parser.h"
#include "../../xyuv/src/to_string.h"
#include "../TestResources.h"

#include <unordered_map>
#include <random>

using namespace xyuv;

class FormatRegression : public ::testing::TestWithParam<std::string> {
public:
    static const uint32_t BASE_W = 8;
    static const uint32_t BASE_H = 8;

    static std::unordered_map<uint64_t, std::unordered_map<float, yuv_image>> map;
    static yuv_image source_image;

    static void SetUpTestCase() {
        source_image = create_yuv_image_444(BASE_W, BASE_H, true, true, true, true );

        std::default_random_engine rng;
        std::uniform_real_distribution<float> dist;

        for (uint32_t y = 0; y < source_image.image_h; y++) {
            for (uint32_t x = 0; x < source_image.image_w; x++) {
                source_image.y_plane.set(x, y, dist(rng));
                source_image.u_plane.set(x, y, dist(rng));
                source_image.v_plane.set(x, y, dist(rng));
                source_image.a_plane.set(x, y, dist(rng));
            }
        }
    }

// Enable a more directed test with more predictable values.
static void test_encode_decode(const ::format_template &format_template, const yuv_image & base_image) {

    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");
    ::chroma_siting chroma_siting;

    chroma_siting.subsampling = format_template.subsampling;
    chroma_siting.u_sample_point = {0,0};
    chroma_siting.v_sample_point = {0,0};

    ::format format = create_format(base_image.y_plane.width(), base_image.y_plane.height(), format_template, conversion_matrix, chroma_siting);
    ::frame frame = encode_frame(base_image, format);

    yuv_image decoded_img = decode_frame(frame);


    // Compare surfaces
    if (!format.channel_blocks[channel::Y].samples.empty()) {
        SCOPED_TRACE("Y");
        auto & sample0 = format.channel_blocks[channel::Y].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(base_image.y_plane, decoded_img.y_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::U].samples.empty()) {
        SCOPED_TRACE("U");
        auto & sample0 = format.channel_blocks[channel::U].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(base_image.u_plane, decoded_img.u_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::V].samples.empty()) {
        SCOPED_TRACE("V");
        auto & sample0 = format.channel_blocks[channel::V].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(base_image.v_plane, decoded_img.v_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::A].samples.empty()) {
        SCOPED_TRACE("A");
        auto & sample0 = format.channel_blocks[channel::A].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(base_image.a_plane, decoded_img.a_plane, 1.0f / max);
    }
}

static void compare_surfaces(const surface<float>& expected, const surface<float>& observed, float abs_error) {
    ASSERT_EQ(expected.width(), observed.width());
    ASSERT_EQ(expected.height(), observed.height());

    for (uint32_t x = 0; x < expected.width(); x++) {
        for (uint32_t y = 0; y < expected.height(); y++) {
            SCOPED_TRACE("(" + to_string(x) + ", " + to_string(y) + ")");
            EXPECT_NEAR(expected.at(x, y), observed.at(x, y), abs_error);
        }
    }
}


};

std::unordered_map<uint64_t, std::unordered_map<float, yuv_image>> FormatRegression::map;
yuv_image FormatRegression::source_image;


TEST_P(FormatRegression, FromYUVAndBackAgain) {
    SCOPED_TRACE(GetParam());
    ::format_template     format_template = Resources::config().get_format_template(GetParam());
    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");
    ::chroma_siting chroma_siting;

    try {
        auto sitings = Resources::config().get_chroma_sitings(format_template.subsampling);
        std::string siting_string = *sitings.begin();
        chroma_siting = Resources::config().get_chroma_siting(siting_string);
    } catch (std::exception & e) {
        FAIL() << e.what();
        return;
    }

    yuv_image expected_image = source_image;

    if (!is_444(chroma_siting.subsampling)) {
        expected_image = down_sample(expected_image, chroma_siting);
    }

    ::format format = create_format(expected_image.image_w, expected_image.image_h, format_template, conversion_matrix, chroma_siting);
    ::frame frame = encode_frame(expected_image, format);

    yuv_image observed_image = decode_frame(frame);

    // Compare surfaces
    if (!format.channel_blocks[channel::Y].samples.empty()) {
        SCOPED_TRACE("Y");
        auto & sample0 = format.channel_blocks[channel::Y].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.y_plane, observed_image.y_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::U].samples.empty()) {
        SCOPED_TRACE("U");
        auto & sample0 = format.channel_blocks[channel::U].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.u_plane, observed_image.u_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::V].samples.empty()) {
        SCOPED_TRACE("V");
        auto & sample0 = format.channel_blocks[channel::V].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.v_plane, observed_image.v_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::A].samples.empty()) {
        SCOPED_TRACE("A");
        auto & sample0 = format.channel_blocks[channel::A].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.a_plane, observed_image.a_plane, 1.0f / max);
    }
}


TEST_P(FormatRegression, ToFileAndBackAgain) {
    SCOPED_TRACE(GetParam());
    ::format_template     format_template = Resources::config().get_format_template(GetParam());
    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");
    ::chroma_siting chroma_siting;

    try {
        auto sitings = Resources::config().get_chroma_sitings(format_template.subsampling);
        std::string siting_string = *sitings.begin();
        chroma_siting = Resources::config().get_chroma_siting(siting_string);
    } catch (std::exception & e) {
        FAIL() << e.what();
        return;
    }

    yuv_image expected_image = source_image;

    if (!is_444(chroma_siting.subsampling)) {
        expected_image = down_sample(expected_image, chroma_siting);
    }

    // Encode frame
    ::format format = create_format(expected_image.image_w, expected_image.image_h, format_template, conversion_matrix, chroma_siting);
    ::frame frame = encode_frame(expected_image, format);

    // Store frame.
    std::stringstream sstream(std::ios::binary | std::ios::in | std::ios::out);
    write_frame( sstream, frame );

    // Read frame
    ::frame loaded_frame;
    read_frame(&loaded_frame, sstream);

    // Decode frame
    yuv_image observed_image = decode_frame(loaded_frame);

    // Compare surfaces
    if (!format.channel_blocks[channel::Y].samples.empty()) {
        SCOPED_TRACE("Y");
        auto & sample0 = format.channel_blocks[channel::Y].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.y_plane, observed_image.y_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::U].samples.empty()) {
        SCOPED_TRACE("U");
        auto & sample0 = format.channel_blocks[channel::U].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.u_plane, observed_image.u_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::V].samples.empty()) {
        SCOPED_TRACE("V");
        auto & sample0 = format.channel_blocks[channel::V].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.v_plane, observed_image.v_plane, 1.0f / max);
    }

    if (!format.channel_blocks[channel::A].samples.empty()) {
        SCOPED_TRACE("A");
        auto & sample0 = format.channel_blocks[channel::A].samples.front();
        uint64_t max = ((0x1ull << (sample0.integer_bits)) - 1) << sample0.fractional_bits;
        compare_surfaces(expected_image.a_plane, observed_image.a_plane, 1.0f / max);
    }
}


INSTANTIATE_TEST_CASE_P(, FormatRegression, ::testing::ValuesIn(Resources::get_all_formats()));





