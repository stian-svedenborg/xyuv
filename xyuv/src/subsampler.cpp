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
#include <xyuv/yuv_image.h>
#include "assert.h"
#include <cmath>

namespace xyuv {

yuv_image up_sample(const yuv_image &yuva_in) {

    // If input is already 444, simply copy it.
    if (yuva_in.siting.subsampling.macro_px_w == 1 && yuva_in.siting.subsampling.macro_px_h == 1) {
        return yuva_in;
    }

    chroma_siting siting;
    siting.subsampling.macro_px_w = 1;
    siting.subsampling.macro_px_h = 1;
    siting.u_sample_point = {0, 0};
    siting.u_sample_point = {0, 0};

    yuv_image result = create_yuv_image(
            yuva_in.image_w,
            yuva_in.image_h,
            siting,
            !yuva_in.y_plane.empty(),
            !yuva_in.u_plane.empty(),
            !yuva_in.v_plane.empty(),
            !yuva_in.a_plane.empty()
    );

    const xyuv::subsampling &subsampling = yuva_in.siting.subsampling;

    bool has_u = !yuva_in.u_plane.empty();
    bool has_v = !yuva_in.v_plane.empty();

    if (has_u || has_v) {

        uint32_t subsampled_w = (yuva_in.image_w + subsampling.macro_px_w - 1) / subsampling.macro_px_w;
        uint32_t subsampled_h = (yuva_in.image_h + subsampling.macro_px_h - 1) / subsampling.macro_px_h;

        // Upsample: For each u/v sample, populate the affected pixels in the result-surface.
        for (uint32_t y = 0; y < subsampled_h; y++) {
            for (uint32_t x = 0; x < subsampled_w; x++) {
                // TODO: Determine whether interpolation is appropriate here.
                // (It probably is.)

                pixel_quantum u_sample = 0;
                pixel_quantum v_sample = 0;
                if (has_u) u_sample = yuva_in.u_plane.get(x, y);
                if (has_v) v_sample = yuva_in.v_plane.get(x, y);

                for (uint32_t b_y = 0; b_y < yuva_in.siting.subsampling.macro_px_h; b_y++) {
                    uint32_t target_y = y * yuva_in.siting.subsampling.macro_px_h + b_y;
                    if (target_y < result.image_h) {
                        for (uint32_t b_x = 0; b_x < yuva_in.siting.subsampling.macro_px_w; b_x++) {
                            uint32_t target_x = x * yuva_in.siting.subsampling.macro_px_w + b_x;
                            if (target_x < result.image_w) {
                                if (has_u) result.u_plane.set(target_x, target_y, u_sample);
                                if (has_v) result.v_plane.set(target_x, target_y, v_sample);
                            }
                        }
                    }
                }
            }
        }
    }

    // Simply copy remaining planes.
    result.y_plane = yuva_in.y_plane;
    result.a_plane = yuva_in.a_plane;

    return result;
}

yuv_image down_sample(const yuv_image &yuva_in, const chroma_siting &siting) {

    // Check if the current subsampling equals the target subsampling, if so short circut
    if (yuva_in.siting == siting) {
        return yuva_in;
    }

    // If the image is already downsampled, upsample it first.
    if (yuva_in.siting.subsampling.macro_px_w > 1 || yuva_in.siting.subsampling.macro_px_h > 1) {
        return down_sample(up_sample(yuva_in), siting);
    }

    yuv_image result = create_yuv_image(
            yuva_in.image_w,
            yuva_in.image_h,
            siting,
            !yuva_in.y_plane.empty(),
            !yuva_in.u_plane.empty(),
            !yuva_in.v_plane.empty(),
            !yuva_in.a_plane.empty()
    );

    bool has_u = !yuva_in.u_plane.empty();
    bool has_v = !yuva_in.v_plane.empty();

    if (has_u || has_v) {

        uint32_t subsampled_w = (yuva_in.image_w + siting.subsampling.macro_px_w - 1) / siting.subsampling.macro_px_w;
        uint32_t subsampled_h = (yuva_in.image_h + siting.subsampling.macro_px_h - 1) / siting.subsampling.macro_px_h;

        // Downsample: For each u/v sample in the target, sample the affected pixels in the source-surface.
        for (uint32_t y = 0; y < subsampled_h; y++) {
            for (uint32_t x = 0; x < subsampled_w; x++) {
                pixel_quantum u_sample = 0.0f;
                pixel_quantum v_sample = 0.0f;

                // The center of the left-topmost pixel in the block has coordinate 0.0 and step by 1.0 to the center of the next
                // pixel in each dimension.
                // Downsampling is calculated as the weighted average of the (<= four) pixels (inside the block) closest to the sampling point.
                // For future reference: http://www.pcmag.com/encyclopedia/term/57460/chroma-subsampling
                for (uint32_t b_y = 0; b_y < siting.subsampling.macro_px_h; b_y++) {
                    float dy_u = 1.0f - std::fabs(siting.u_sample_point.second - b_y);
                    float dy_v = 1.0f - std::fabs(siting.v_sample_point.second - b_y);

                    if (dy_u < 0.0f && dy_v < 0.0f) {
                        continue;
                    }

                    uint32_t source_y = y * siting.subsampling.macro_px_h + b_y;
                    source_y = std::min(source_y,
                                        yuva_in.image_h - 1); // Clamp to edge TODO: check that this behaves as expected

                    for (uint32_t b_x = 0; b_x < siting.subsampling.macro_px_w; b_x++) {
                        float dx_u = 1.0f - std::fabs(siting.u_sample_point.first - b_x);
                        float dx_v = 1.0f - std::fabs(siting.v_sample_point.first - b_x);

                        if (dx_u <= 0.0f && dx_v <= 0.0f) {
                            continue;
                        }

                        uint32_t source_x = x * siting.subsampling.macro_px_w + b_x;
                        source_x = std::min(source_x, yuva_in.image_w - 1); // Clamp to edge TODO: check this.

                        // TODO: Perform some testing here to ensure that we don't hit any funky floating point errors.
                        if (has_u && dx_u > 0.0f) {
                            u_sample += dx_u * dy_u * yuva_in.u_plane.get(source_x, source_y);
                        }
                        if (has_v && dx_v > 0.0f) {
                            v_sample += dx_v * dy_v * yuva_in.v_plane.get(source_x, source_y);
                        }
                    }
                }

                if (has_u) result.u_plane.set(x, y, u_sample);
                if (has_v) result.v_plane.set(x, y, v_sample);
            }
        }
    }

    // Simply copy y and alpha
    result.y_plane = yuva_in.y_plane;
    result.a_plane = yuva_in.a_plane;

    return result;
}

} // namespace xyuv