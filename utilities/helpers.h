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

#pragma once

#include <xyuv.h>
#include <xyuv/structures/chroma_siting.h>
#include <xyuv/structures/conversion_matrix.h>
#include <xyuv/config_manager.h>

class Helpers {
public:
    static xyuv::frame AddHeader(const xyuv::format_template &fmt_template,
                                 const xyuv::chroma_siting &siting,
                                 const xyuv::conversion_matrix &matrix,
                                 uint32_t image_w,
                                 uint32_t image_h,
                                 std::istream &input_stream
    );

    // Write a frame to file, infering the mode from the file suffix.
    static void WriteFrame(const xyuv::frame &frame, const std::string &out_filename, bool split_planes);

    static xyuv::frame LoadXYUVFile(const std::string &infile_name);

    static xyuv::frame LoadConvertRGBImage(const xyuv::format_template &fmt_template, const xyuv::conversion_matrix &matrix, const xyuv::chroma_siting &siting, const std::string & infile_name);

    static void WriteMetadata(const xyuv::frame &frame, const std::string &raw_out_filename);

    static xyuv::frame LoadConvertFrame(const xyuv::format &, const std::string &infile_name);
    static xyuv::frame LoadConvertFrame( const xyuv::format & format, const std::vector<std::string> & infiles );

    static int GetAdaptedConsoleWidth();

    static std::string FormatString(uint32_t indent, uint32_t max_width, const std::string &str);

    static void PrintHelpSection(const std::string &short_arg, const std::string &long_arg,
                                 const std::string &helpstring);

    static std::string GetSuffix(const std::string &filename);

    //! Split the filename into the stem and the suffix, e.g. /x/y/z.bar is split into "/x/y/z" and ".bar"
    static std::pair<std::string, std::string> SplitSuffix(const std::string &filename);

    static std::string ToLower(const std::string& str);

    static void PrintAllFormats(const xyuv::config_manager &manager);
};

