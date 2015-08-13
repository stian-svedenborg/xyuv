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

#include <xyuv/structures/format.h>
#include <cstring>

namespace xyuv {

// These structures are pods.
bool operator==(const subsampling &lhs, const subsampling &rhs) {
    return memcmp(&lhs, &rhs, sizeof(subsampling)) == 0;
}

bool operator==(const chroma_siting &lhs, const chroma_siting &rhs) {
    return memcmp(&lhs, &rhs, sizeof(chroma_siting)) == 0;
}

bool operator==(const conversion_matrix &lhs, const conversion_matrix &rhs) {
    return memcmp(&lhs, &rhs, sizeof(conversion_matrix)) == 0;
}

bool operator==(const plane &lhs, const plane &rhs) {
    return memcmp(&lhs, &rhs, sizeof(plane)) == 0;
}

bool operator==(const sample &lhs, const sample &rhs) {
    return memcmp(&lhs, &rhs, sizeof(sample)) == 0;
}

// Structs containing class types.

bool operator==(const channel_block &lhs, const channel_block &rhs) {
    if (lhs.samples.size() != rhs.samples.size()
        || lhs.h != rhs.h
        || lhs.w != rhs.h
            ) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.samples.size(); i++) {
        if (!(lhs.samples[i] == rhs.samples[i])) return false;
    }
    return true;
}

bool operator==(const format &lhs, const format &rhs) {
    bool equalThusFar = lhs.size == rhs.size
                        && lhs.image_w == rhs.image_w
                        && lhs.image_h == rhs.image_h;

    if (!equalThusFar) return false;

    equalThusFar = lhs.fourcc == rhs.fourcc
                   && lhs.origin == rhs.origin
                   && lhs.planes.size() == rhs.planes.size()
                   && lhs.chroma_siting == rhs.chroma_siting;

    if (!equalThusFar) return false;

    for (std::size_t i = 0; i < lhs.planes.size(); i++) {
        if (!(lhs.planes[i] == rhs.planes[i])) {
            return false;
        }
    }

    for (std::size_t i = 0; i < lhs.channel_blocks.size(); i++) {
        if (!(lhs.channel_blocks[i] == rhs.channel_blocks[i])) {
            return false;
        }
    }

    // Now, finally, we can say, with confidence, the two formats are equal:
    return true;
}

} // namespace xyuv