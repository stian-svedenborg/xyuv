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

#include <xyuv/yuv_image.h>
#include <xyuv.h>
#include "../xyuv/src/to_string.h"
#include "TestResources.h"
#include <random>

#include <gtest/gtest.h>

using namespace xyuv;

class FormatsBase : public ::testing::TestWithParam<std::string> {
protected:
    static std::vector<std::string> list_all_format_templates() {
        std::vector<std::string> formats;
        for (auto & fmt : Resources::config().get_format_templates()) {
            formats.push_back(fmt.first);
        }
        return formats;
    }
};

class Formats : public FormatsBase {

};

static void randomize_plane( surface<pixel_quantum> & surf ) {
    std::mt19937 rng;
    std::uniform_real_distribution<float> rfg(0.0f, 1.0f);

    for (auto & value : surf) {
        value = rfg(rng);
    }
}

yuv_image create_test_image(const ::chroma_siting & chroma_siting) {
    yuv_image image = create_yuv_image(chroma_siting.subsampling.macro_px_w * 3,
                                       chroma_siting.subsampling.macro_px_h * 3,
                                       chroma_siting,
                                       true, true, true, true
    );

    randomize_plane(image.y_plane);
    randomize_plane(image.u_plane);
    randomize_plane(image.v_plane);
    randomize_plane(image.a_plane);

    return image;
}

static void compare_surfaces(const surface<float>& expected, const surface<float>& observed ) {
    ASSERT_EQ(expected.width(), observed.width());
    ASSERT_EQ(expected.height(), observed.height());

    for (uint32_t x = 0; x < expected.width(); x++) {
        for (uint32_t y = 0; y < expected.height(); y++) {
            SCOPED_TRACE("(" + to_string(x) + ", " + to_string(y) + ")");
            EXPECT_FLOAT_EQ(expected.at(x, y), observed.at(x, y));
        }
    }
}

static void compare_yuv_images(const yuv_image & expected, const yuv_image & observed) {
    ASSERT_EQ(expected.image_w, observed.image_w );
    ASSERT_EQ(expected.image_h, observed.image_h );
    ASSERT_EQ(expected.siting, observed.siting);

    compare_surfaces(expected.y_plane, observed.y_plane);
    compare_surfaces(expected.u_plane, observed.u_plane);
    compare_surfaces(expected.v_plane, observed.v_plane);
    compare_surfaces(expected.a_plane, observed.a_plane);
}

TEST_P(Formats, subsampling) {
    // Chroma sitings must ensure the following.
    // - No-op on Y and A plane.
    // - If the operation is expected_sub_sampled -> expected_444 -> observed_subsampled -> observed_444
    //   then expected should equal observed.

    ::chroma_siting chroma_siting = Resources::config().get_chroma_siting(GetParam());

    yuv_image expected_sub = create_test_image(chroma_siting);
    yuv_image expected_444 = up_sample(expected_sub);
    yuv_image observed_sub = down_sample(expected_444, chroma_siting);
    yuv_image observed_444 = up_sample(observed_sub);

    compare_yuv_images(expected_sub, observed_sub);
    compare_yuv_images(expected_444, observed_444);

}

INSTANTIATE_TEST_CASE_P(, Formats, ::testing::Values("422", "420", "411", "410"));