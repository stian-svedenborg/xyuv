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

#pragma once

namespace  xyuv {

struct conversion_matrix;
struct yuv_image;


/** @brief Interface to integrate third party rgb image libraries with xYUV.*/
class rgb_image {
public:
    void from_yuv_image(const xyuv::yuv_image &image_in, const xyuv::conversion_matrix &conversion_matrix);

    yuv_image to_yuv_image(const xyuv::conversion_matrix &conversion_matrix) const;

protected:
    virtual void xyuv_from_yuv_image_444(const xyuv::yuv_image &yuv_image_444,
                                         const xyuv::conversion_matrix &conversion_matrix) = 0;

    virtual yuv_image xyuv_to_yuv_image_444(const xyuv::conversion_matrix &conversion_matrix) const = 0;
};

} // namespace xyuv