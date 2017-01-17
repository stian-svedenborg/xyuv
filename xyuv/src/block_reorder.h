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

#ifndef CROSSYUV_BLOCK_REORDER_H
#define CROSSYUV_BLOCK_REORDER_H

#include <cstdint>
namespace xyuv {

    struct format;

    uint32_t get_block_order_offset(uint32_t block_x, uint32_t block_y, const ::block_order & block_order);
    std::pair<uint32_t,uint32_t > get_block_order_coords(uint32_t block_x, uint32_t block_y, const ::block_order & block_order);

    bool needs_reorder(const xyuv::format & format );
    void reorder_transform(uint8_t *frame_base_ptr, const xyuv::plane &plane);
    void reorder_inverse(uint8_t *frame_base_ptr, const xyuv::plane &plane);

}
#endif //CROSSYUV_BLOCK_REORDER_H
