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
#include <xyuv/frame.h>
#include <xyuv/structures/format_template.h>
#include <xyuv.h>
#include <xyuv/yuv_image.h>
#include "../xyuv/src/config_parser.h"
#include "TestResources.h"

using namespace xyuv;

//! In this test we want to stress the Toplevel API and ensure the results are sane.
class TopLevelAPITest : public ::testing::Test {
public:
    format_template fmt_444, fmt_420;
    chroma_siting siting_444, siting_420;
    conversion_matrix rgb_matrix;

protected:
    virtual void SetUp() override {
        fmt_444 = Resources::get().config().get_format_template("AYUV");
        fmt_420 = Resources::get().config().get_format_template("IMC1");

        siting_444 = Resources::get().config().get_chroma_siting("444");
        siting_420 = Resources::get().config().get_chroma_siting("420");

        rgb_matrix = Resources::get().config().get_conversion_matrix("bt601");
    }
};

void ASSERT_SIZE(const yuv_image & image, uint32_t width, uint32_t height) {
    ASSERT_EQ(width,  image.y_plane.width());
    ASSERT_EQ(height, image.y_plane.height());
    ASSERT_EQ(width,  image.a_plane.width());
    ASSERT_EQ(height, image.a_plane.height());

    uint32_t subsampled_width = (width + image.siting.subsampling.macro_px_w -1) / image.siting.subsampling.macro_px_w;
    uint32_t subsampled_height = (height + image.siting.subsampling.macro_px_h -1) / image.siting.subsampling.macro_px_h;
    ASSERT_EQ(subsampled_width, image.u_plane.width());
    ASSERT_EQ(subsampled_height, image.u_plane.height());
    ASSERT_EQ(subsampled_width, image.v_plane.width());
    ASSERT_EQ(subsampled_height, image.v_plane.height());

    ASSERT_EQ(width, image.image_w);
    ASSERT_EQ(height, image.image_h);
}



TEST_F(TopLevelAPITest, ScaleYUVImage) {
    // Simple scaling.
    // - All 444
    // - All aligned to pow 2.
    yuv_image image = create_yuv_image(4, 4, siting_444, true, true, true, true);
    yuv_image scaled_image = scale_yuv_image(image, 8, 8);

    ASSERT_SIZE(scaled_image, 8, 8);

    image = scaled_image;
    scaled_image = scale_yuv_image(image, 4, 4);

    ASSERT_SIZE(scaled_image, 4, 4);

    // Intermediate scaling.
    // - Siting 420
    // - All aligned to pow 2.

    image = create_yuv_image(4, 4, siting_420, true, true, true, true);
    scaled_image = scale_yuv_image(image, 8, 8);

    ASSERT_SIZE(scaled_image, 8, 8);

    image = scaled_image;
    scaled_image = scale_yuv_image(image, 4, 4);

    ASSERT_SIZE(scaled_image, 4, 4);

    // More difficult scaling.
    // - Siting 420
    // - Non-power of 2

    image = create_yuv_image(7, 13, siting_420, true, true, true, true);
    scaled_image = scale_yuv_image(image, 8, 17);

    ASSERT_SIZE(scaled_image, 8, 17);

    image = scaled_image;
    scaled_image = scale_yuv_image(image, 2, 2);

    ASSERT_SIZE(scaled_image, 2, 2);

    // Cornercases
    // Down to 1x1 px
    image = create_yuv_image(4, 4, siting_420, true, true, true, true);
    scaled_image = scale_yuv_image(image, 1, 1);

    ASSERT_SIZE(scaled_image, 1, 1);

    // Up from 1x1 px.
    image = scaled_image;
    scaled_image = scale_yuv_image(image, 4, 4);

    ASSERT_SIZE(scaled_image, 4, 4);
}

static void fill(surface<pixel_quantum> & surf, pixel_quantum val) {
    for (pixel_quantum & px : surf) {
        px = val;
    }
}

static yuv_image create_image(uint32_t w, uint32_t h, const chroma_siting & siting) {
    yuv_image image = create_yuv_image(w, h, siting, true, true, true, true);
    fill(image.y_plane, 0.0f);
    fill(image.u_plane, 0.0f);
    fill(image.v_plane, 0.0f);
    fill(image.a_plane, 1.0f);
    return image;
}

TEST_F(TopLevelAPITest, EncodeFrame) {
    // For now, just exercise the API and see if anything segfaults.

    // Simple:
    // - Input 444,
    // - Dimensions agree.
    yuv_image image = create_image(4, 4, siting_444);
    ::format format = create_format(image.image_w, image.image_h, fmt_444, rgb_matrix, siting_444);

    ::frame frame = encode_frame(image, format);

    format = create_format(image.image_w*5, image.image_h/3, fmt_444, rgb_matrix, siting_444);

    frame = encode_frame(image, format);

    // More difficult, 420
    image = create_image(4, 4, siting_420);
    format = create_format(image.image_w, image.image_h, fmt_420, rgb_matrix, siting_420);

    frame = encode_frame(image, format);

    format = create_format(image.image_w*5, image.image_h/3, fmt_420, rgb_matrix, siting_420);

    frame = encode_frame(image, format);

}
