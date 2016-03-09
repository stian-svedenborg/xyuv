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

#include <xyuv/yuv_image.h>
#include <string.h>
#include "xyuv/frame.h"
#include "assert.h"
#include "utility.h"
#include "block_reorder.h"


static inline void copy_bits(uint8_t * dst_base, uint32_t dst_bit_offset, const uint8_t * src_base, uint32_t src_bit_offset, uint32_t size_in_bits) {
    for (uint32_t i = 0; i < size_in_bits; i++) {
        xyuv::set_bit(dst_base, dst_bit_offset + i, xyuv::get_bit(src_base, src_bit_offset + i));
    }
}

namespace xyuv {

    // Calculate the position of this block.
    inline uint32_t get_block_order_offset(uint32_t block_x, uint32_t block_y, const ::block_order & block_order) {
        uint32_t xval = 0;
        uint32_t yval = 0;
        for (uint32_t i = 0; i < 32; ++i) {
            uint32_t x_mask = static_cast<uint32_t>(0x1ul << block_order.x_mask[i]);
            bool x_bit_set = (x_mask & block_x) != 0u;
            xval |= static_cast<uint32_t>(x_bit_set) << i;

            uint32_t y_mask = static_cast<uint32_t>(0x1ul << block_order.y_mask[i]);
            bool y_bit_set = (y_mask & block_y) != 0u;
            yval |= static_cast<uint32_t>(y_bit_set) << i;

        }

        uint32_t offset = xval ^ yval;
        return offset;
    }


    inline std::pair<uint32_t,uint32_t > get_block_order_coords(uint32_t block_x, uint32_t block_y, const ::block_order & block_order) {

        uint32_t  offset = get_block_order_offset(block_x, block_y, block_order);

        auto p = std::make_pair(offset % block_order.mega_block_width, offset / block_order.mega_block_width);
        return p;
    }


    bool needs_reorder(const xyuv::format & format ) {
        for (auto & plane : format.planes ) {
            if (plane.block_order.mega_block_height != 1
                || plane.block_order.mega_block_width != 1) {
                return true;
            }
        }
        return false;
    }

    void reorder_transform( uint8_t * frame_base_ptr, const xyuv::plane &plane) {

        if (plane.block_order.mega_block_height == 1
                && plane.block_order.mega_block_width == 1) {
            return;
        }

        uint8_t * plane_base_ptr = frame_base_ptr + plane.base_offset;

        uint32_t mega_block_line_stride = plane.block_order.mega_block_width*plane.block_stride;
        XYUV_ASSERT((mega_block_line_stride % 8) == 0
                    && "Assert that block line_stride is full multiple of bytes.");
        mega_block_line_stride /= 8;

        uint32_t mega_block_size = mega_block_line_stride*plane.block_order.mega_block_height;



        uint32_t width_in_possible_blocks = plane.line_stride*8 / plane.block_stride;
        uint32_t width_in_macro_blocks = width_in_possible_blocks / plane.block_order.mega_block_width;

        uint32_t height_in_possible_blocks_lines = static_cast<uint32_t>(plane.size / plane.line_stride);
        uint32_t height_in_macro_blocks = height_in_possible_blocks_lines / plane.block_order.mega_block_height;

        std::unique_ptr<uint8_t> temp_plane {new uint8_t[plane.size]};

        for (uint32_t block_y = 0; block_y < height_in_macro_blocks; block_y++) {
            for (uint32_t block_x = 0; block_x < width_in_macro_blocks; block_x++) {
                uint8_t * block_base = temp_plane.get() + block_y*(plane.block_order.mega_block_height*plane.line_stride) + block_x*mega_block_size;

                for (uint32_t y = 0; y < plane.block_order.mega_block_height; y++) {
                    const uint8_t * src_line = plane_base_ptr
                                               + (block_y*plane.block_order.mega_block_height + y)*plane.line_stride;
                    for (uint32_t x = 0; x < plane.block_order.mega_block_width; x++) {
                        auto internal_coord = get_block_order_coords(x, y, plane.block_order);
                        // Get destination line of micro block.
                        uint8_t* dst_line = block_base + internal_coord.second * mega_block_line_stride;
                        copy_bits(dst_line, internal_coord.first*plane.block_stride, src_line , (block_x*plane.block_order.mega_block_width + x)*plane.block_stride, plane.block_stride );
                    }
                }
            }
        }
        memcpy(plane_base_ptr, temp_plane.get(), plane.size);
    }


    void reorder_inverse( uint8_t * frame_base_ptr, const xyuv::plane &plane) {

        if (plane.block_order.mega_block_height == 1
            && plane.block_order.mega_block_width == 1) {
            return;
        }

        uint8_t * plane_base_ptr = frame_base_ptr + plane.base_offset;

        uint32_t mega_block_line_stride = plane.block_order.mega_block_width*plane.block_stride;

        XYUV_ASSERT((mega_block_line_stride % 8) == 0
                    && "Assert that block line_stride is full multiple of bytes.");
        mega_block_line_stride /= 8;

        uint32_t mega_block_size = mega_block_line_stride*plane.block_order.mega_block_height;


        uint32_t width_in_possible_blocks = plane.line_stride*8 / plane.block_stride;
        uint32_t width_in_macro_blocks = width_in_possible_blocks / plane.block_order.mega_block_width;

        uint32_t height_in_possible_blocks_lines = static_cast<uint32_t>(plane.size / plane.line_stride);
        uint32_t height_in_macro_blocks = height_in_possible_blocks_lines / plane.block_order.mega_block_height;

        std::unique_ptr<uint8_t> temp_plane { new uint8_t[plane.size] };

        for (uint32_t block_y = 0; block_y < height_in_macro_blocks; block_y++) {
            for (uint32_t block_x = 0; block_x < width_in_macro_blocks; block_x++) {

                const uint8_t * block_base = plane_base_ptr + block_y*(plane.block_order.mega_block_height*plane.line_stride) + block_x*mega_block_size;

                for (uint32_t y = 0; y < plane.block_order.mega_block_height; y++) {
                    // Linear output line
                    uint8_t * dst_line = temp_plane.get() + (block_y*plane.block_order.mega_block_height + y)*plane.line_stride;
                    for (uint32_t x = 0; x < plane.block_order.mega_block_width; x++) {
                        auto internal_coord = get_block_order_coords(x, y, plane.block_order);
                        // Get destination line of micro block.
                        const uint8_t* blocked_line = block_base + internal_coord.second * mega_block_line_stride;
                        copy_bits(dst_line, (block_x*plane.block_order.mega_block_width + x)*plane.block_stride, blocked_line , internal_coord.first*plane.block_stride, plane.block_stride );
                    }
                }

            }
        }
        memcpy(plane_base_ptr, temp_plane.get(), plane.size);
    }

}
