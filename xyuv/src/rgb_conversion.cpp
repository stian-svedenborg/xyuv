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

#include "rgb_conversion.h"
#include "xyuv/structures/conversion_matrix.h"
#include "xyuv/structures/color.h"
#include "xyuv/rgb_image.h"
#include "xyuv/yuv_image.h"
#include "utility.h"

#include <algorithm>

namespace xyuv {

void to_rgb(rgb_color *rgb, const yuv_color &yuv_in, const conversion_matrix &matrix, bool has_y, bool has_u, bool has_v) {
    yuv_color yuv = yuv_in;

    // Assume, 0.0 is zero i.e. no contribution.
    // Scale to [min, max], and shift minimum to min.
    if (has_y) {
        yuv.y *= matrix.y_range.second - matrix.y_range.first;
        yuv.y += matrix.y_range.first;
    }
    if (has_u) {
        yuv.u *= matrix.u_range.second - matrix.u_range.first;
        yuv.u += matrix.u_range.first;
    }
    if (has_v) {
        yuv.v *= matrix.v_range.second - matrix.v_range.first;
        yuv.v += matrix.v_range.first;
    }

    rgb->r = matrix.yuv_to_rgb[0] * yuv.y + matrix.yuv_to_rgb[1] * yuv.u + matrix.yuv_to_rgb[2] * yuv.v;
    rgb->g = matrix.yuv_to_rgb[3] * yuv.y + matrix.yuv_to_rgb[4] * yuv.u + matrix.yuv_to_rgb[5] * yuv.v;
    rgb->b = matrix.yuv_to_rgb[6] * yuv.y + matrix.yuv_to_rgb[7] * yuv.u + matrix.yuv_to_rgb[8] * yuv.v;
    rgb->a = yuv.a;


    rgb->r = clamp(0.0f, 1.0f, rgb->r);
    rgb->g = clamp(0.0f, 1.0f, rgb->g);
    rgb->b = clamp(0.0f, 1.0f, rgb->b);
    rgb->a = clamp(0.0f, 1.0f, rgb->a);
}

void to_yuv(yuv_color *yuv, const rgb_color &rgb, const conversion_matrix &matrix) {
    yuv->y = matrix.rgb_to_yuv[0] * rgb.r + matrix.rgb_to_yuv[1] * rgb.g + matrix.rgb_to_yuv[2] * rgb.b;
    yuv->u = matrix.rgb_to_yuv[3] * rgb.r + matrix.rgb_to_yuv[4] * rgb.g + matrix.rgb_to_yuv[5] * rgb.b;
    yuv->v = matrix.rgb_to_yuv[6] * rgb.r + matrix.rgb_to_yuv[7] * rgb.g + matrix.rgb_to_yuv[8] * rgb.b;
    yuv->a = rgb.a;

    // Shift minimum to 0.
    yuv->y -= matrix.y_range.first;
    yuv->u -= matrix.u_range.first;
    yuv->v -= matrix.v_range.first;

    // Scale to [0.0, 1.0]
    yuv->y /= matrix.y_range.second - matrix.y_range.first;
    yuv->u /= matrix.u_range.second - matrix.u_range.first;
    yuv->v /= matrix.v_range.second - matrix.v_range.first;

    yuv->y = clamp(0.0f, 1.0f, yuv->y);
    yuv->u = clamp(0.0f, 1.0f, yuv->u);
    yuv->v = clamp(0.0f, 1.0f, yuv->v);
    yuv->a = clamp(0.0f, 1.0f, yuv->a);
}

void yuv_image_to_rgb(rgb_image *rgbImage_out, const xyuv::yuv_image &yuv_image,
                      const xyuv::conversion_matrix &matrix) {
    rgbImage_out->from_yuv_image(yuv_image, matrix);
}

yuv_image rgb_to_yuv_image(const rgb_image &rgbImage_in, const xyuv::conversion_matrix &matrix) {
    return rgbImage_in.to_yuv_image(matrix);
}

} // namespace xyuv