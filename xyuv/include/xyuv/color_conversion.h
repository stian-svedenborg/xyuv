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

#pragma once

namespace xyuv {

struct rgb_color;
struct yuv_color;
struct conversion_matrix;

/** \brief Convert a single yuv_color point to rgb.
 * \par rgb[out] output color.
 * \par yuv[in] input color.
 * \par matrix[in] conversion matrix to use for the conversion.
 * \par has_y[in]  true if the source color uses the y component.
 * \par has_u[in]  true if the source color uses the u component.
 * \par has_v[in]  true if the source color uses the v component.
 */
void to_rgb(rgb_color *rgb, const yuv_color &yuv, const conversion_matrix &matrix, bool has_y, bool has_u, bool has_v);

/** \brief Convert a single rgb_color point to yuv.
 * \par yuv[out] output color.
 * \par rgb[in] input color.
 * \par matrix[in] conversion matrix to use for the conversion.
 */
void to_yuv(yuv_color *yuv, const rgb_color &rgb, const conversion_matrix &matrix);

} // namespace xyuv