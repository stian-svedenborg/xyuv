/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2021 Stian Valentin Svedenborg
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
#include <xyuv/large_buffer.h>
#include <xyuv/frame.h>
#include <iostream>
#include "helpers.h"
#include <xyuv/format_json.h>
#include <map>
#include <functional>

#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
#include "external/magick_format_rw.h"
#endif

#if defined(USE_LIBPNG) && USE_LIBPNG
#include "external/libpng_format_rw.h"
#endif



void WriteHexFile(const std::string & filename, const uint8_t * data, uint64_t size, uint64_t memory_map_offset = 0);

std::string Helpers::GetSuffix(const std::string & path) {
    return path.substr(path.rfind('.', std::string::npos), std::string::npos);
}

std::pair<std::string, std::string> Helpers::SplitSuffix(const std::string &path) {
    std::size_t dot_pos = path.rfind('.');
    std::size_t slash_pos = path.rfind('/');
    if (dot_pos < slash_pos) {
        return std::make_pair(path, std::string(""));
    }
    else {
        return std::make_pair(path.substr(0, dot_pos), path.substr(dot_pos));
    }

}

static void write_frame_using_xyuv(const xyuv::frame & frame, const std::string & out_stem, const std::string& suffix, bool split_planes) {
    if (split_planes) {
        throw std::logic_error("Suffix .xyuv does not support split-planes.");
    }
    auto out_filename = out_stem + suffix;
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_frame(fout, frame);
}

static void WriteRawFile(const std::string & out_filename, const std::uint8_t* buffer, const std::uint64_t buffersize) {
    std::ofstream fout(out_filename, std::ios::binary);
    if (!fout) {
        throw std::runtime_error("Could not open output file: '" + out_filename + "' for writing");
    }
    xyuv::write_large_buffer(fout, reinterpret_cast<const char *>(buffer), buffersize);
}

static void write_frame_raw(const xyuv::frame & frame, const std::string & out_stem, const std::string & suffix, bool split_planes) {
    if (split_planes) {
        for (int i = 0; i < frame.format.planes.size(); i++) {
            auto outfilename = out_stem + "-p" + std::to_string(i) + suffix;
            WriteRawFile(outfilename, frame.data.get() + frame.format.planes[i].base_offset, frame.format.planes[i].size);
        }
    }
    else {
        WriteRawFile(out_stem + suffix, frame.data.get(), frame.format.size);
    }
}

static void write_frame_hex(const xyuv::frame & frame, const std::string & out_stem, const std::string & suffix, bool split_planes) {
    if (split_planes) {
        for (int i = 0; i < frame.format.planes.size(); i++) {
            auto outfilename = out_stem + "-p" + std::to_string(i) + suffix;
            WriteHexFile(outfilename, frame.data.get() + frame.format.planes[i].base_offset, frame.format.planes[i].size, frame.format.planes[i].base_offset);
        }
    }
    else {
        WriteHexFile(out_stem + suffix, frame.data.get(), frame.format.size);
    }
}

void Helpers::WriteFrame(const xyuv::frame &frame, const std::string &out_filename, bool split_planes) {
    static std::map<std::string, std::function<void(const xyuv::frame &, const std::string &, const std::string &, bool)>> map {
            {".xyuv", write_frame_using_xyuv },
            {".hex", write_frame_hex },
            {".bin", write_frame_raw },
            {".raw", write_frame_raw },
            {".yuv", write_frame_raw },
#if defined(USE_LIBPNG) && USE_LIBPNG
            {".png", WriteConvertFrame_libpng },
#elif defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".png", WriteConvertFrame_imagemagick },
#endif
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
            {".jpg", WriteConvertFrame_imagemagick },
            {".gif", WriteConvertFrame_imagemagick },
            {".tga", WriteConvertFrame_imagemagick },
            {".bmp", WriteConvertFrame_imagemagick },
#endif
    };

    auto file_parts = Helpers::SplitSuffix(out_filename);
    const auto & out_stem = file_parts.first;
    const auto & suffix = file_parts.second;

    // If the suffix is in out map, use it
    auto it = map.find(Helpers::ToLower(suffix));
    if (it != map.end()) {
        (it->second)(frame, out_stem, suffix, split_planes);
    }
    else {
#if defined(USE_IMAGEMAGICK) && USE_IMAGEMAGICK
        std::cout << "[Warning]: Unrecognized file suffix '" + suffix + "' trying to pass it to ImageMagick." << std::endl;
        WriteConvertFrame_imagemagick(frame, out_stem, suffix, split_planes);
#else
        throw std::runtime_error("[Error]: Unrecognized file suffix '" + suffix + "' aborting.");
#endif
    }
}

void Helpers::WriteMetadata(const xyuv::frame & frame, const std::string & raw_out_filename) {
    std::string metadata_filename = raw_out_filename + ".json";

    std::ofstream fout(metadata_filename);

    if (!fout) {
        throw std::runtime_error("Could not open file '" + metadata_filename + "'.");
    }

    std::string json_metadata = xyuv::format_to_json(frame.format);

    fout << json_metadata << std::endl;

    if (!fout) {
        throw std::runtime_error("Error occured while writing '" + metadata_filename + "'.");
    }

    fout.close();
}
