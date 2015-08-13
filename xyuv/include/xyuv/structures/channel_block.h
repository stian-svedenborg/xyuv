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
#include "sample.h"
#include <vector>

namespace xyuv {

//! \brief struct defining how to read samples for one channel.
//! \details This defines the smallest cluster of pixels that must be present in "one go", i.e. some formats have "blocks"
//!     that tightly pack together multiple pixels. The geometry of such a cluster is defined in this struct.
//!     \paragraph This struct has no knowledge of which channel it is, as that is determined by it's position in the
//!     parent array.
//! TODO: Add link to explanation.
struct channel_block {
    //! Width and height of a block of pixels measure in this channel.
    uint16_t w, h;
    //! Sample descriptors for this block
    std::vector<sample> samples;
};

bool operator==(const channel_block &lhs, const channel_block &rhs);

} // namespace xyuv
