/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Stian Valentin Svedenborg
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


#ifndef CROSSYUV_BLOCK_ORDER_H
#define CROSSYUV_BLOCK_ORDER_H

#include <cstring>

struct block_order {
    enum : uint8_t {
        NOT_USED = 32,
    };

    block_order() {
        memset(x_mask, NOT_USED, sizeof(x_mask));
        memset(y_mask, NOT_USED, sizeof(y_mask));
    }

    // Width and height of mega_block (in atomic blocks)
    uint32_t mega_block_width = 1, mega_block_height = 1;
    uint8_t x_mask[32], y_mask[32];
};

bool operator==(const block_order & lhs, const block_order & rhs);

#endif //CROSSYUV_BLOCK_ORDER_H
