/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Stian Valentin Svedenborg
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


#include <fstream>
#include <functional>
#include <xyuv/frame.h>
#include <xyuv/large_buffer.h>
#include <iostream>
#include <map>
#include "helpers.h"
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
#include "external/magick_format_rw.h"
#endif

#if defined(USE_LIBPNG) && USE_LIBPNG
#include "external/libpng_format_rw.h"
#endif

std::string get_suffix(const std::string & path);


static xyuv::frame LoadConvertFrame_xyuv(const xyuv::format & format, const std::string & infile_name ) {
    std::ifstream fin(infile_name, std::ios::binary);

    if (!fin) {
        throw std::runtime_error("Could not open input file: '" + infile_name + "'");
    }

    xyuv::frame frame;
    xyuv::read_frame(&frame, fin);
    return frame;
}

static xyuv::frame LoadConvertFrame_raw(const xyuv::format & format, const std::string & infile_name ) {
    std::ifstream fin(infile_name, std::ios::binary);

    if (!fin) {
        throw std::runtime_error("Could not open input file: '" + infile_name + "'");
    }

    auto frame = xyuv::create_frame(format, nullptr, 0);

    uint64_t bytes_read = xyuv::read_large_buffer(fin, reinterpret_cast<char*>(frame.data.get()), format.size);

    if (bytes_read != format.size) {
        throw std::runtime_error("Error loading frame data, only " + std::to_string(bytes_read) + " bytes read, expected " + std::to_string(format.size));
    }

    return frame;
}

std::vector<uint8_t> LoadHexFile(const std::string & filename);

static xyuv::frame LoadConvertFrame_hex(const xyuv::format & format, const std::string & infile_name ) {

    auto frame = xyuv::create_frame(format, nullptr, 0);

    std::vector<uint8_t> buffer = LoadHexFile(infile_name);

    if (buffer.size() < format.size) {
        throw std::runtime_error("Error loading frame data, only " + std::to_string(buffer.size()) + " bytes read, expected " + std::to_string(format.size));
    }

    memcpy(frame.data.get(), buffer.data(), format.size );

    return frame;
}

xyuv::frame Helpers::LoadConvertFrame( const xyuv::format & format, const std::string & infile_name ) {
    static std::map<std::string, std::function<xyuv::frame(const xyuv::format &, const std::string &)>> map{
            {".xyuv", LoadConvertFrame_xyuv},
            {".bin", LoadConvertFrame_raw},
            {".raw", LoadConvertFrame_raw},
            {".yuv", LoadConvertFrame_raw},
#if defined(USE_LIBPNG) && USE_LIBPNG
            {".png", LoadConvertFrame_libpng },
#elif defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".png", LoadConvertFrame_imagemagick},
#endif
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".jpg", LoadConvertFrame_imagemagick},
            {".gif", LoadConvertFrame_imagemagick},
            {".tga", LoadConvertFrame_imagemagick},
            {".bmp", LoadConvertFrame_imagemagick},
#endif
            {".hex", LoadConvertFrame_hex},
            {".dump", LoadConvertFrame_hex},
    };

    std::string suffix = Helpers::ToLower(Helpers::GetSuffix(infile_name));

    // If the suffix is in out map, use it
    auto it = map.find(suffix);
    if (it != map.end()) {
        return (it->second)(format, infile_name);
    }
    else {
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
        std::cout << "[Warning]: Unrecognized file suffix '" + suffix + "' for input file '" + infile_name + "' trying to pass it to ImageMagick." << std::endl;
        return LoadConvertFrame_imagemagick(format, infile_name);
#else
        throw std::runtime_error("[Error]: Unrecognized file suffix '" + suffix + "' aborting.");
#endif
    }
}

xyuv::frame Helpers::LoadXYUVFile(const std::string &infile_name) {
    std::ifstream fin(infile_name, std::ios::binary);

    if (!fin) {
        throw std::runtime_error("Could not open input file: '" + infile_name + "'");
    }

    xyuv::frame frame;
    xyuv::read_frame(&frame, fin);
    return frame;
}

xyuv::frame Helpers::LoadConvertRGBImage(const xyuv::format_template &fmt_template, const xyuv::conversion_matrix &matrix, const xyuv::chroma_siting &siting, const std::string & infile_name) {
    static std::map<std::string, std::function<xyuv::frame(const xyuv::format_template &, const xyuv::conversion_matrix &, const xyuv::chroma_siting &, const std::string &)>> map{
#if defined(USE_LIBPNG) && USE_LIBPNG
            {".png", LoadConvertRGBFrame_libpng },
#elif defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".png", LoadConvertRGBFrame_imagemagick},
#endif
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".jpg", LoadConvertRGBFrame_imagemagick},
            {".gif", LoadConvertRGBFrame_imagemagick},
            {".tga", LoadConvertRGBFrame_imagemagick},
            {".bmp", LoadConvertRGBFrame_imagemagick},
#endif
    };

    std::string suffix = Helpers::ToLower(Helpers::GetSuffix(infile_name));

    // If the suffix is in out map, use it
    auto it = map.find(suffix);
    if (it != map.end()) {
        return (it->second)(fmt_template, matrix, siting, infile_name);
    }
    else {
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
        std::cout << "[Warning]: Unrecognized file suffix '" + suffix + "' for input file '" + infile_name + "' trying to pass it to ImageMagick." << std::endl;
        return LoadConvertRGBFrame_imagemagick(fmt_template, matrix, siting, infile_name);
#else
        throw std::runtime_error("[Error]: Unrecognized file suffix '" + suffix + "' aborting.");
#endif
    }
}