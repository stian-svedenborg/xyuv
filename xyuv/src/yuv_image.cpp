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

#include <xyuv/structures/chroma_siting.h>
#include "xyuv/yuv_image.h"
#include "xyuv.h"

namespace xyuv {

yuv_image create_yuv_image(
        uint32_t image_w,
        uint32_t image_h,
        const xyuv::chroma_siting &siting,
        bool has_Y,
        bool has_U,
        bool has_V,
        bool has_A
) {
    yuv_image result;
    result.image_w = image_w;
    result.image_h = image_h;

    result.siting = siting;

    // Subsampled dimensions.
    uint32_t subsampled_w = (image_w + siting.subsampling.macro_px_w - 1) / siting.subsampling.macro_px_w;
    uint32_t subsampled_h = (image_h + siting.subsampling.macro_px_h - 1) / siting.subsampling.macro_px_h;

    if (has_Y) result.y_plane.resize(image_w, image_h);
    if (has_U) result.u_plane.resize(subsampled_w, subsampled_h);
    if (has_V) result.v_plane.resize(subsampled_w, subsampled_h);
    if (has_A) {
        result.a_plane.resize(image_w, image_h);
        result.a_plane.fill(1.0f);
    }

    return std::move(result);
}

yuv_image create_yuv_image_444(
        uint32_t image_w,
        uint32_t image_h,
        bool has_Y,
        bool has_U,
        bool has_V,
        bool has_A
) {
    // The simplest siting possible.
    chroma_siting chroma_siting_444 = {{1,    1},
                                       {0.0f, 0.0f},
                                       {0.0f, 0.0f}};

    return create_yuv_image(image_w, image_h, chroma_siting_444, has_Y, has_U, has_V, has_A);
}


yuv_image scale_yuv_image(const yuv_image &yuv_in, uint32_t new_width, uint32_t new_height) {
    if (yuv_in.image_w == new_width && yuv_in.image_h == new_height) {
        return yuv_in;
    }

    yuv_image temp;

    if (!is_444(yuv_in.siting.subsampling)) {
        temp = up_sample(yuv_in);
    } else {
        // Ensure we are working on a copy.
        temp = yuv_in;
    }

    // DO the scaling.
    if (!temp.y_plane.empty()) temp.y_plane.scale(new_width, new_height);
    if (!temp.u_plane.empty()) temp.u_plane.scale(new_width, new_height);
    if (!temp.v_plane.empty()) temp.v_plane.scale(new_width, new_height);
    if (!temp.a_plane.empty()) temp.a_plane.scale(new_width, new_height);

    // Set the scaled sizes.
    temp.image_w = new_width;
    temp.image_h = new_height;

    if (!is_444(yuv_in.siting.subsampling)) {
        return down_sample(temp, yuv_in.siting);
    }

    return temp;
}

} // namespace xyuv