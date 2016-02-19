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

#pragma once
#include "constants.h"
#include "block_order.h"
#include <string>

namespace xyuv {

struct plane_template {
    std::string base_offset_expression;
    std::string line_stride_expression;
    std::string plane_size_expression;
    uint32_t block_stride; // The stride of a block in this plane.
    interleave_pattern interleave_mode;
    ::block_order block_order;
};

struct plane {
    uint64_t base_offset;
    uint64_t size;
    uint32_t line_stride;
    uint32_t block_stride; // The stride of a block in this plane in bits.
    interleave_pattern interleave_mode;
    ::block_order block_order;
};

bool operator==(const plane &lhs, const plane &rhs);

} // namespace xyuv