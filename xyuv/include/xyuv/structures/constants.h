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

namespace xyuv {

//! \brief Enum describing the origin [posision of pixel (0, 0)] of the image.
//! This is used when mapping from in memory data to xyuv::yuv_image which is always UPPER_LEFT.
enum class image_origin {
    UPPER_LEFT = 0,
    LOWER_LEFT = 1,
};

//! \brief Enum mapping an channel to an integer.
//! \details It is wrapped in a struct to enforce scoped usage. i.e. you must write
//! \code{.cpp} channel::Y \endcode
struct channel { ;
    enum {
        Y = 0, U = 1, V = 2, A = 3
    };
};

//! Enum defining possible interleave modes.
enum class interleave_pattern : uint8_t {
    //! \brief The plane is not interleaved.
    //! \details The scan lines in the plane are placed one after the other in a natural order: {0, 1, 2, 3, ...}.
    NO_INTERLEAVING = 0,

    //! \brief The plane is interleaved, odd scan lines before even.
    //! \details All odd scan lines appear before the even scan lines: {1, 3, 5, ... , 0, 2, 4, ...}.
    INTERLEAVE_1_3_5__0_2_4 = 1,

    //! \brief The plane is interleaved, even scan lines before odd.
    //! \details All even scan lines appear before the odd scan lines: {0, 2, 4, ... , 1, 3, 5, ...}.
    INTERLEAVE_0_2_4__1_3_5 = 2,
};

} // namespace xyuv