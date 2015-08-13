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

#include <xyuv.h>
#include <xyuv/frame.h>
#include <xyuv/yuv_image.h>

namespace xyuv {

xyuv::frame convert_frame(const xyuv::frame &frame_in, const format &new_format) {
    // First, check if this is a no-op.
/*    if (frame_in.format == new_format) {
        return frame_in;
    }
*/
    xyuv::yuv_image temporary_image = decode_frame(frame_in);
    return encode_frame(temporary_image, new_format);
}


xyuv::frame read_frame_from_rgb_image(const rgb_image &rgbImage_in, const format &new_format) {
    yuv_image temporary_image = rgb_to_yuv_image(rgbImage_in, new_format.conversion_matrix);
    return encode_frame(temporary_image, new_format);
}

void write_frame_to_rgb_image(rgb_image *rgbImage_out, const xyuv::frame &frame_in) {
    yuv_image temporary_image = decode_frame(frame_in);
    yuv_image_to_rgb(rgbImage_out, temporary_image, frame_in.format.conversion_matrix);
}

} // namespace xyuv