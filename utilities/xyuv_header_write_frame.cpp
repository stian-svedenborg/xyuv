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

#include <Magick++/Image.h>
#include <fstream>
#include <xyuv/imagemagick/magick_wrapper.h>
#include <xyuv/large_buffer.h>
#include <xyuv/frame.h>
#include <iostream>
#include "XYUVHeader.h"

void WriteHexFile(const std::string & filename, const uint8_t * data, uint64_t size);

std::string get_suffix(const std::string & path) {
    return path.substr(path.rfind('.', std::string::npos), std::string::npos);
}

static void write_frame_using_image_magick(const xyuv::frame &frame, const std::string & out_filename) {
    Magick::Image image(Magick::Geometry(frame.format.image_w,frame.format.image_h,0,0), Magick::ColorRGB(0.0,0.0,0.0));

    xyuv::magick_wrapper wrapper(image);
    xyuv::write_frame_to_rgb_image(&wrapper, frame);

    image.write(out_filename);
}

static void write_frame_using_xyuv(const xyuv::frame & frame, const std::string & out_filename) {
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_frame(fout, frame);
}

static void write_frame_raw(const xyuv::frame & frame, const std::string & out_filename) {
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_large_buffer(fout, reinterpret_cast<const char*>(frame.data.get()), frame.format.size);
}

static void write_frame_hex(const xyuv::frame & frame, const std::string & out_filename) {
    WriteHexFile(out_filename, frame.data.get(), frame.format.size);
}

void XYUVHeader::WriteFrame(const xyuv::frame & frame, const std::string & out_filename) {
    static std::map<std::string, std::function<void(const xyuv::frame &, const std::string &)>> map {
            {".xyuv", write_frame_using_xyuv },
            {".hex", write_frame_hex },
            {".bin", write_frame_raw },
            {".raw", write_frame_raw },
            {".yuv", write_frame_raw },
            {".png", write_frame_using_image_magick },
            {".jpg", write_frame_using_image_magick },
            {".gif", write_frame_using_image_magick },
            {".tga", write_frame_using_image_magick },
            {".bmp", write_frame_using_image_magick },
    };

    std::string suffix = get_suffix(out_filename);

    // Make it lowercase.
    for (char & c : suffix ) {
        c = static_cast<char>(std::tolower(c));
    }

    // If the suffix is in out map, use it
    auto it = map.find(suffix);
    if (it != map.end()) {
        (it->second)(frame, out_filename);
    }
    else {
        std::cout << "[Warning]: Unrecognized file suffix '" + suffix + "' trying to pass it to imagemagick." << std::endl;
        write_frame_using_image_magick(frame, out_filename);
    }
}
