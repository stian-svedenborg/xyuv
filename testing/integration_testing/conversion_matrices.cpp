/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Stian Valentin Svedenborg
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

#include "../TestResources.h"
#include "xyuv/structures/conversion_matrix.h"
#include "../../xyuv/src/rgb_conversion.h"
#include "../../xyuv/src/to_string.h"
#include <gtest/gtest.h>
#include <xyuv/structures/color.h>


class ConversionMatrixRegression : public ::testing::TestWithParam<std::string> {
public:
    static const uint32_t BASE_W = 8;
    static const uint32_t BASE_H = 8;



    static void test_encode_decode(const std::string & name) {

        xyuv::conversion_matrix conversion_matrix = Resources::get().config().get_conversion_matrix(name);

        xyuv::rgb_color colors[] = {
                {1.0, 1.0, 1.0, 1.0},
                {1.0, 0.33, 0.5, 1.0},
                {.1234, 0.542, 0.7568, 1.0},
                {.9999, 0.999, 0.99, 1.0},
                {.0001, 0.333333333, 0.456, 1.0},
                {.99999999, 0.11111111, 0.22222222, 1.0},
                {0.0, 0.0, 0.0, 1.0},
                {0.0, 0.0, 1.0, 1.0},
                {0.0, 1.0, 0.0, 1.0},
                {1.0, 0.0, 0.0, 1.0},
        };

        SCOPED_TRACE(name);
        for (auto & rgb : colors) {
            xyuv::yuv_color yuv0;
            xyuv::to_yuv(&yuv0, rgb, conversion_matrix);

            xyuv::rgb_color rgb0;
            xyuv::to_rgb(&rgb0, yuv0, conversion_matrix, true, true, true);

            SCOPED_TRACE("(" + xyuv::to_string(rgb.r) + ", " + xyuv::to_string(rgb.r) + ", " + xyuv::to_string(rgb.r) + ")");
            ASSERT_NEAR(rgb0.r, rgb.r, 1.0f/(1<<15));
            ASSERT_NEAR(rgb0.g, rgb.g, 1.0f/(1<<15));
            ASSERT_NEAR(rgb0.b, rgb.b, 1.0f/(1<<15));
        }

    }
};

TEST_P(ConversionMatrixRegression, Inversion) {
    test_encode_decode(GetParam());
}

INSTANTIATE_TEST_CASE_P(, ConversionMatrixRegression, ::testing::ValuesIn(Resources::get().get_all_conversion_matrices()));
