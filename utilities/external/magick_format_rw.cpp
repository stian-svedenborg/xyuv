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

#include "magick_format_rw.h"
#include <xyuv.h>
#include <xyuv/frame.h>
#include <Magick++/Geometry.h>
#include <Magick++/Image.h>
#include <xyuv/external/magick_wrapper.h>

xyuv::frame LoadConvertFrame_imagemagick(const xyuv::format & format, const std::string & infile_name ) {
    Magick::Image image(infile_name);
    xyuv::magick_wrapper wrapper(image);
    return xyuv::read_frame_from_rgb_image(wrapper, format);
}

xyuv::frame LoadConvertRGBFrame_imagemagick(const xyuv::format_template &fmt_template, const xyuv::conversion_matrix &matrix, const xyuv::chroma_siting &siting, const std::string & infile_name) {
    Magick::Image image(infile_name);
    xyuv::magick_wrapper wrapper(image);
    xyuv::format format = xyuv::create_format(wrapper.columns(), wrapper.rows(), fmt_template, matrix, siting);
    return xyuv::read_frame_from_rgb_image(wrapper, format);
}

void WriteConvertFrame_imagemagick(const xyuv::frame &frame, const std::string & out_filename) {
    Magick::Image image(Magick::Geometry(frame.format.image_w,frame.format.image_h,0,0), Magick::ColorRGB(0.0,0.0,0.0));

    xyuv::magick_wrapper wrapper(image);
    xyuv::write_frame_to_rgb_image(&wrapper, frame);

    image.write(out_filename);
}

void Display_imagemagick(const xyuv::frame & frame) {
    Magick::Image image(Magick::Geometry(1,1,0,0), Magick::ColorRGB(0.0,0.0,0.0));

    xyuv::magick_wrapper wrapper(image);
    xyuv::write_frame_to_rgb_image(&wrapper, frame);

    image.display();
}