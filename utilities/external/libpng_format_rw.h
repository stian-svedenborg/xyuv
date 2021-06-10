/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2021 Stian Valentin Svedenborg
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

#ifndef CROSSYUV_LIBPNG_FORMAT_RW_H
#define CROSSYUV_LIBPNG_FORMAT_RW_H

#include <string>
namespace xyuv {
    struct frame;
    struct format;
    struct format_template;
    struct chroma_siting;
    struct conversion_matrix;
}

xyuv::frame LoadConvertFrame_libpng(const xyuv::format & format, const std::vector<std::string>& infile_name );
xyuv::frame LoadConvertRGBFrame_libpng(const xyuv::format_template &fmt_template, const xyuv::conversion_matrix &matrix, const xyuv::chroma_siting &siting, const std::string & infile_name);

void WriteConvertFrame_libpng(const xyuv::frame &frame, const std::string &out_stem, const std::string& suffix, bool split_planes);

#endif //CROSSYUV_LIBPNG_FORMAT_RW_H
