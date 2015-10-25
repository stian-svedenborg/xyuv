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

#include <xyuv/structures/color.h>
#include "TestResources.h"
#include "../xyuv/src/rgb_conversion.h"

#include <gtest/gtest.h>

using namespace xyuv;

TEST(RGBTest, EncodeDecode) {

    ::conversion_matrix conversion_matrix = Resources::config().get_conversion_matrix("bt601");

    const rgb_color rgb_expected = { 0.333f, 1.0f, 0.0f, 0.5f};

    yuv_color yuv;
    to_yuv(&yuv, rgb_expected, conversion_matrix);

    rgb_color rgb_observed;
    to_rgb(&rgb_observed, yuv, conversion_matrix, true, true, true);

    ASSERT_NEAR(rgb_expected.r, rgb_observed.r, 0.00001f);
    ASSERT_NEAR(rgb_expected.g, rgb_observed.g, 0.00001f);
    ASSERT_NEAR(rgb_expected.b, rgb_observed.b, 0.00001f);
    ASSERT_NEAR(rgb_expected.a, rgb_observed.a, 0.00001f);

}
