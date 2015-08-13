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

#include <xyuv/structures/conversion_matrix.h>
#include "magick_wrapper.h"
#include "../../xyuv/src/config_parser.h"
#include <xyuv/yuv_image.h>
#include <xyuv.h>
#include <Magick++.h>
#include "../TestResources.h"
#include <xyuv/structures/format.h>
#include <xyuv/frame.h>

using namespace xyuv;

class ImageMagickWrapperTest : public ::testing::TestWithParam<std::string> {
protected:
    void read_write_store_image(const std::string & fmt_name);
};

static void compare_colors(const Magick::ColorRGB & expected, const Magick::ColorRGB & observed, bool expected_has_alpha, bool observed_has_alpha ) {
    ASSERT_NEAR(expected.red(), observed.red(), 1.1f/(0x1<<8));
    ASSERT_NEAR(expected.green(), observed.green(), 1.1f/(0x1<<8));
    ASSERT_NEAR(expected.blue(), observed.blue(), 1.1f/(0x1<<8));
    if (expected_has_alpha) {
        ASSERT_NEAR(expected.alpha(), observed.alpha(), 1.1f / (0x1 << 8));
    }
    else if (observed_has_alpha) {
        ASSERT_NEAR(1.0f, observed.alpha(), 1.1f /(0x1 << 8));
    }
}


TEST_F(ImageMagickWrapperTest, RGB_to_and_from_YUV) {
    Magick::Image image_expected = Resources::get_lena512();
    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");

    const magick_wrapper img(image_expected);

    ::yuv_image yuv_image = rgb_to_yuv_image( img, conversion_matrix );

    Magick::Image image_observed(Magick::Geometry(yuv_image.image_w, yuv_image.image_h), Magick::Color("black"));
    magick_wrapper img2(image_observed);
    yuv_image_to_rgb(&img2, yuv_image, conversion_matrix);

    ASSERT_EQ(image_expected.matte(), image_observed.matte());
#if 1
    const Magick::PixelPacket * expected_pixels = image_expected.getConstPixels(0,0, image_expected.columns(), image_expected.rows());
    const Magick::PixelPacket * observed_pixels = image_observed.getConstPixels(0,0, image_observed.columns(), image_observed.rows());


    for (uint32_t y = 0; y < image_expected.rows(); y++) {
        for ( uint32_t x = 0; x < image_expected.columns(); x++) {
            compare_colors(
                    Magick::Color(expected_pixels[image_expected.columns()*y + x]),
                    Magick::Color(observed_pixels[image_observed.columns()*y + x]),
                    image_expected.matte(),
                    image_observed.matte()
            );
        }
    }
#endif
//    image_observed.write("testing/integration_testing/test_data/lena_out.png");
}

void ImageMagickWrapperTest::read_write_store_image(const std::string & fmt_name) {

    Magick::Image image_expected = Resources::get_default();
    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");
    ::format_template fmt_template = Resources::config().get_format_template(fmt_name);
    ::chroma_siting chroma_siting; // Pick a valid chroma siting.
    try {
        auto sitings = Resources::config().get_chroma_sitings(fmt_template.subsampling);
        std::string siting_string = *sitings.begin();
        chroma_siting = Resources::config().get_chroma_siting(siting_string);
    } catch (std::exception & e) {
        FAIL() << e.what();
        return;
    }

    const magick_wrapper img(image_expected);

    // Prepare format
    format fmt = create_format(
            image_expected.columns(),
            image_expected.rows(),
            fmt_template,
            conversion_matrix,
            chroma_siting );

    ::frame frame = read_frame_from_rgb_image(img, fmt);


    Magick::Image image_observed(Magick::Geometry(image_expected.columns(), image_expected.rows()), Magick::Color("black"));
    magick_wrapper img2(image_observed);


    write_frame_to_rgb_image(&img2, frame);

#if 0
    const Magick::PixelPacket * expected_pixels = image_expected.getConstPixels(0,0, image_expected.columns(), image_expected.rows());
    const Magick::PixelPacket * observed_pixels = image_observed.getConstPixels(0,0, image_observed.columns(), image_observed.rows());


    for (uint32_t y = 0; y < image_expected.rows(); y++) {
        for ( uint32_t x = 0; x < image_expected.columns(); x++) {
            compare_colors(
                    Magick::Color(expected_pixels[image_expected.columns()*y + x]),
                    Magick::Color(observed_pixels[image_observed.columns()*y + x]),
                    image_expected.matte(),
                    image_observed.matte()
            );
        }
    }
#endif
    image_observed.write("testing/integration_testing/test_data/output/observed_image_via_" + fmt_name + ".png");
}



TEST_P(ImageMagickWrapperTest, ReadWriteStoreImage) {
    read_write_store_image(GetParam());
}

INSTANTIATE_TEST_CASE_P(, ImageMagickWrapperTest, ::testing::ValuesIn(Resources::get_all_formats()));

#include <iostream>


TEST_F(ImageMagickWrapperTest, MinimalTest) {
    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");

    // Prepare a single pixel value.
    Magick::ColorRGB expected_color;
    expected_color.red( 0.333 );
    expected_color.green( 0.653 );
    expected_color.blue( 0.80 );
    expected_color.alpha(0.5);

    // Prepare a simple image.
    Magick::Image image_expected(Magick::Geometry(1,1,0,0), expected_color);
    // Activate alpha.
    image_expected.matte(true);
    // Set once more after activating alpha
    *image_expected.setPixels(0,0,1,1) = expected_color;
    image_expected.syncPixels();

    // Create a blank image to store the result.
    Magick::Image image_observed(Magick::Geometry(1,1,0,0), Magick::Color("black"));
    image_observed.matte(true);

    // Convert to and from a yuv_image.
    const magick_wrapper proxy(image_expected);
    yuv_image yuv_representation = rgb_to_yuv_image(proxy, conversion_matrix );

    magick_wrapper proxy_out(image_observed);
    yuv_image_to_rgb(&proxy_out, yuv_representation, conversion_matrix );

    // Compare
    Magick::Color observed_color = *Magick::Pixels(image_observed).get(0,0,1,1);
    compare_colors(expected_color, observed_color, true, true);
}
