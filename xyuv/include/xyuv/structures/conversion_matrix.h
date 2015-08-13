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

#include <utility>
#include <cstdint>

namespace xyuv {

//! struct defining all parameters needed to convert between the rgb and yuv color-spaces.
//! \details Or any other color-space representable by the same parameters.
struct conversion_matrix {
    //! 3x3 row major matrix to convert an rgb pixel (range [0.0, 1.0]) to yuv (range y_range, u_range, v_range respectively).
    float rgb_to_yuv[9];
    //! 3x3 row major matrix to convert a yuv pixel (range y_range, u_range, v_range respectively) to rgb (range [0.0, 1.0]).
    float yuv_to_rgb[9];

    //! \brief Range of the output of the xyuv::rgb_to_yuv conversion matrix.
    //! \details These fields are used to normalize the internal representation of a yuv pixel to [0.0, 1.0].
    std::pair<float, float> y_range, u_range, v_range;

    //! \brief Min-max values for the packed (i.e. encoded unorm) value)
    //! \details This is here to support studio range values. The calculation is rather straight forward.
    //!     Say you are to restrict the number of 8 bit encoded luma values to [16, 235], then y_packed range
    //!     should be [16/255, 235/255] (as 255 is the largest value representable in 8 bits). Note that in doing so
    //!     you also support 10 bit 8.2 Unorm encodings of the same ranges. In fact any format that has the same
    //!     integer-bits is good to go. If the number of integer bits change you are in deeper trouble and must create
    //!     a separate conversion matrix object.
    std::pair<float, float> y_packed_range, u_packed_range, v_packed_range;

};

bool operator==(const conversion_matrix &lhs, const conversion_matrix &rhs);

} // namespace xyuv