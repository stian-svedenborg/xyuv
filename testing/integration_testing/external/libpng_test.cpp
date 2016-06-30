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

#include <gtest/gtest.h>

#include <xyuv/structures/conversion_matrix.h>
#include <xyuv/structures/format.h>
#include <xyuv/frame.h>
#include <xyuv/external/libpng_wrapper.h>
#include "../../xyuv/src/config_parser.h"
#include <xyuv/yuv_image.h>
#include <xyuv.h>
#include "../../TestResources.h"
#include "../../xyuv/src/to_string.h"

using namespace xyuv;

class LibPNGWrapperTest : public ::testing::TestWithParam<std::string> {
protected:
    void CompareImages(const xyuv::libpng_wrapper & image_expected, const xyuv::libpng_wrapper & image_observed, pixel_quantum abs_diff ) {
        for (uint32_t y = 0; y < image_expected.rows(); y++) {
            for ( uint32_t x = 0; x < image_expected.columns(); x++) {
                SCOPED_TRACE("X: " + to_string(x) + " Y: " + to_string(y));
                xyuv::rgb_color expected = image_expected.get_pixel(x,y);
                xyuv::rgb_color observed = image_observed.get_pixel(x,y);
                EXPECT_NEAR(expected.r, observed.r, abs_diff);
                EXPECT_NEAR(expected.g, observed.g, abs_diff);
                EXPECT_NEAR(expected.b, observed.b, abs_diff);
                EXPECT_NEAR(expected.a, observed.a, abs_diff);
            }
        }
    }
};


TEST_F(LibPNGWrapperTest, RgbToYuvAndBack) {
    xyuv::libpng_wrapper image_expected(Resources::get().get_png_path(Resources::TestImage::LENA));

    ::conversion_matrix conversion_matrix = Resources::get().config().get_conversion_matrix("bt601");
    ::yuv_image yuv_image = rgb_to_yuv_image( image_expected, conversion_matrix );
    xyuv::pixel_quantum abs_diff = (conversion_matrix.u_range.second - conversion_matrix.u_range.first)*255.0f;

    xyuv::libpng_wrapper image_observed;
    image_observed.from_yuv_image(yuv_image, conversion_matrix);

    CompareImages(image_expected, image_observed, abs_diff);
}

TEST_F(LibPNGWrapperTest, LoadStoreReload) {
    std::string out_file = "./testing/integration_testing/test_data/output/libpng_tmp.png";
    xyuv::libpng_wrapper image_expected(Resources::get().get_png_path(Resources::TestImage::LENA));
    image_expected.save_png_to_file(out_file);
    xyuv::libpng_wrapper image_observed(out_file);

    CompareImages(image_expected, image_observed, 0.0f);
}

TEST_F(LibPNGWrapperTest, AssignmentTest) {
    xyuv::libpng_wrapper image_expected(Resources::get().get_png_path(Resources::TestImage::LENA));

    xyuv::libpng_wrapper copy_constructed{*static_cast<const xyuv::libpng_wrapper*>(&image_expected)};
    CompareImages(image_expected, copy_constructed, 0.0f);

    xyuv::libpng_wrapper copy_assigned(1, 1);
    copy_assigned = *static_cast<const xyuv::libpng_wrapper*>(&image_expected);
    CompareImages(image_expected, copy_assigned, 0.0f);

    xyuv::libpng_wrapper move_assigned;
    move_assigned = std::move(copy_assigned);
    CompareImages(image_expected, move_assigned, 0.0f);

    xyuv::libpng_wrapper move_constructed {std::move(move_assigned)};
    CompareImages(image_expected, move_constructed, 0.0f);


}