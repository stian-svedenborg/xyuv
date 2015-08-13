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
#include <magick_wrapper.h>
#include "xyuv/structures/conversion_matrix.h"
#include "xyuv/structures/color.h"
#include "../rgb_conversion.h"
#include <Magick++.h>

namespace xyuv {

magick_wrapper::magick_wrapper(Magick::Image &image) :
        image(image) { }

static inline void get_yuv_color(yuv_color &color, const yuv_image &img_444, uint32_t x, uint32_t y) {
    if (!img_444.y_plane.empty()) color.y = img_444.y_plane.get(x, y);
    if (!img_444.u_plane.empty()) color.u = img_444.u_plane.get(x, y);
    if (!img_444.v_plane.empty()) color.v = img_444.v_plane.get(x, y);
    if (!img_444.a_plane.empty()) color.a = img_444.a_plane.get(x, y);
}

void magick_wrapper::xyuv_from_yuv_image_444(const xyuv::yuv_image &yuv_image_444,
                                             const xyuv::conversion_matrix &conversion_matrix) {
    // Resize underlying image to the input image.
    image.resize(Magick::Geometry(yuv_image_444.image_w, yuv_image_444.image_h));

    bool has_a = !yuv_image_444.a_plane.empty();
    image.matte(has_a);

    // Create pixels with default values.
    // Note that this hides the lack of planes.
    rgb_color rgb;
    yuv_color yuv;

    Magick::PixelPacket *pixels = image.setPixels(0, 0, yuv_image_444.image_w, yuv_image_444.image_h);

    for (uint32_t y = 0; y < yuv_image_444.image_h; y++) {
        for (uint32_t x = 0; x < yuv_image_444.image_w; x++) {
            // Convert data.
            get_yuv_color(yuv, yuv_image_444, x, y);
            to_rgb(&rgb, yuv, conversion_matrix);

            // Wrap into ImageMagick ColorRGB
            Magick::ColorRGB magick_rgb;
            magick_rgb.red(rgb.r);
            magick_rgb.green(rgb.g);
            magick_rgb.blue(rgb.b);
            if (has_a) magick_rgb.alpha(rgb.a);

            // Assign pixel.
            pixels[y * image.columns() + x] = magick_rgb;
        }
    }

    // Commit changes to image.
    image.syncPixels();
}

static inline void set_yuv_color(yuv_image &img_444, const yuv_color &color, uint32_t x, uint32_t y) {
    if (!img_444.y_plane.empty()) img_444.y_plane.set(x, y, color.y);
    if (!img_444.u_plane.empty()) img_444.u_plane.set(x, y, color.u);
    if (!img_444.v_plane.empty()) img_444.v_plane.set(x, y, color.v);
    if (!img_444.a_plane.empty()) img_444.a_plane.set(x, y, color.a);
}

yuv_image magick_wrapper::xyuv_to_yuv_image_444(const xyuv::conversion_matrix &conversion_matrix) const {
    // Analyse the conversion matrix to discover if any plane is not present.
    bool has[3] = {false, false, false};
    for (uint32_t i = 0; i < 9; i++) {
        has[i / 3] = has[i / 3] || (conversion_matrix.rgb_to_yuv[i] != static_cast<pixel_quantum>(0));
    }
    bool has_a = image.matte();


    yuv_image image_out = create_yuv_image_444(image.columns(), image.rows(), has[0], has[1], has[2], has_a);

    // Create pixels with default values.
    // Note that this hides the lack of planes.
    rgb_color rgb;
    yuv_color yuv;

    const Magick::PixelPacket *pixels = image.getConstPixels(0, 0, image.columns(), image.rows());

    for (uint32_t y = 0; y < image_out.image_h; y++) {
        for (uint32_t x = 0; x < image_out.image_w; x++) {
            // Fetch color from ImageMagick
            const Magick::ColorRGB magick_rgb = Magick::Color(pixels[y * image.columns() + x]);
            rgb.r = static_cast<float>(magick_rgb.red());
            rgb.g = static_cast<float>(magick_rgb.green());
            rgb.b = static_cast<float>(magick_rgb.blue());
            rgb.a = static_cast<float>(magick_rgb.alpha());

            // Convert to yuv.
            to_yuv(&yuv, rgb, conversion_matrix);

            // Assign pixel
            set_yuv_color(image_out, yuv, x, y);
        }
    }
    // No syncing needed.

    return image_out;
}

} // namespace xyuv