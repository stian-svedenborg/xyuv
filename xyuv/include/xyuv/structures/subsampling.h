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
#include <cstdint>

namespace xyuv {

//! \brief struct defining a subsampling mode.
//! \details i.e. How many luma samples occur per chroma sample.
struct subsampling {
    //! Defines many luma samples (and in what geometry) is used per chroma sample.
    //! \details Internally this is used to validate a xyuv::format_template  against a xyuv::chroma_siting .
    uint8_t macro_px_w, macro_px_h;
};

//! \brief Returns true if the subsampling mode is 444, i.e. One luma sample per chroma sample.
static inline bool is_444(const xyuv::subsampling &subsampling) {
    return subsampling.macro_px_w == 1 && subsampling.macro_px_h == 1;
}

bool operator==(const subsampling & lhs, const subsampling & rhs);

} // namespace xyuv
