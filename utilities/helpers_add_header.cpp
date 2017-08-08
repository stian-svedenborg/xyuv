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

#include "helpers.h"
#include <xyuv/structures/format.h>
#include <xyuv/frame.h>
#include <xyuv/large_buffer.h>

xyuv::frame Helpers::AddHeader(const xyuv::format_template_old & fmt_template,
                                  const xyuv::chroma_siting & siting,
                                  const xyuv::conversion_matrix & matrix,
                                  uint32_t image_w,
                                  uint32_t image_h,
                                  std::istream & input_stream
)
{
    auto format = xyuv::create_format(image_w, image_h, fmt_template, matrix, siting);
    auto frame = xyuv::create_frame(format, nullptr, 0);

    uint64_t bytes_read = xyuv::read_large_buffer(input_stream, reinterpret_cast<char*>(frame.data.get()), format.size);

    if (bytes_read != format.size) {
        throw std::runtime_error("Error loading frame data, only " + std::to_string(bytes_read) + " bytes read, expected " + std::to_string(format.size));
    }

    return frame;
}