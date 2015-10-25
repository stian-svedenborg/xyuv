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

#include "xyuv/quantum.h"

namespace xyuv {

//! \brief Internal struct used to pass around a single yuva color.
//! \details Internally we let all components of yuv colors also go between [0.0, 1.0].
//! This means that opaque black -> (y, u, v, a) == (0, 0, 0, 1.0).
struct yuv_color {
    yuv_color() = default;

    yuv_color(pixel_quantum y, pixel_quantum u, pixel_quantum v, pixel_quantum a) : y(y), u(u), v(v), a(a) { }

    pixel_quantum y = 0.0, u = 0.0, v = 0.0, a = 1.0;
};

//! \brief Internal struct used to pass around a single rgba value.
struct rgb_color {
    rgb_color() = default;

    rgb_color(pixel_quantum r, pixel_quantum g, pixel_quantum b, pixel_quantum a) : r(r), g(g), b(b), a(a) { }

    pixel_quantum r = 0, g = 0, b = 0, a = 1;
};

} // namespace xyuv