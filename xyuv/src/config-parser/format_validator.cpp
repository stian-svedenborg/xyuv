/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Stian Valentin Svedenborg
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


#include "format_validator.h"
#include "xyuv/structures/format_template.h"
#include "xyuv/structures/format.h"
#include "../to_string.h"
#include "../block_reorder.h"
#include <vector>
#include <map>
#include <limits>

namespace xyuv {

// Check whether any of the samples overflow the block or each other
static bool check_sample_overlap(const xyuv::format_template &format_template) {
    std::vector<std::vector<bool>> plane_block;

    for (auto &plane : format_template.planes) {
        plane_block.push_back(std::vector<bool>(plane.block_stride));
    }

    for (auto &channel_block : format_template.channel_blocks) {
        for (auto &sample : channel_block.samples) {
            for (uint32_t i = 0; i < sample.fractional_bits + sample.integer_bits; i++) {
                // Check for overlapping samples.
                if (plane_block[sample.plane][sample.offset + i]) {
                    return false;
                }
                else {
                    plane_block[sample.plane][sample.offset + i] = true;
                }
            }
        }
    }

    return true;
}

static bool check_block_overflow(const xyuv::format_template &format_template) {
    for (auto &channel_block : format_template.channel_blocks) {
        for (auto &sample : channel_block.samples) {
            // Check block overflow.
            if (static_cast<std::size_t>(sample.offset + sample.integer_bits + sample.fractional_bits) >
                format_template.planes[sample.plane].block_stride) {
                return false;
            }
        }
    }
    return true;
}



bool validate_format_template(const xyuv::format_template &format_template) {

    if (!check_block_overflow(format_template)) {
        throw std::logic_error("Block overflow in format template.");
    }

    if (!check_sample_overlap(format_template)) {
        throw std::logic_error("Overlapping samples in format template.");
    }

    return true;
}

    ///////////////////////
    // Validate format
    ///////////////////////
    // Validating formats is a stricter process, and includes line and plane validations in addition to
    // samples.

    // Check whether any of the samples overflow the block or each other
    static bool check_sample_overlap(const xyuv::format &format) {
        std::vector<std::vector<bool>> plane_block;

        for (auto &plane : format.planes) {
            plane_block.push_back(std::vector<bool>(plane.block_stride));
        }

        for (auto &channel_block : format.channel_blocks) {
            for (auto &sample : channel_block.samples) {
                for (uint32_t i = 0; i < sample.fractional_bits + sample.integer_bits; i++) {
                    // Check for overlapping samples.
                    if (plane_block[sample.plane][sample.offset + i]) {
                        return false;
                    }
                    else {
                        plane_block[sample.plane][sample.offset + i] = true;
                    }
                }
            }
        }

        return true;
    }

    static bool check_block_overflow(const xyuv::format &format) {
        for (auto &channel_block : format.channel_blocks) {
            for (auto &sample : channel_block.samples) {
                // Check block overflow.
                if (static_cast<std::size_t>(sample.offset + sample.integer_bits + sample.fractional_bits) >
                    format.planes[sample.plane].block_stride) {
                    return false;
                }
            }
        }
        return true;
    }

    static void check_block_order(const xyuv::format & format) {
        uint8_t i = 0;
        for (auto & plane : format.planes ) {
            uint64_t block_size = static_cast<uint64_t>(plane.block_order.mega_block_width)*plane.block_order.mega_block_height;
            if (block_size > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())) {
                throw std::logic_error("Plane " + to_string(i) + ": block reorder size illegal, mega_block_width * mega_block_height must fit within an unsigned 32 bit integer.");
            }

            if (block_size == 1) {
                return;
            }

            // The reordering is expected to be a bijection, thus the sum of all transformed blocks should be same as the sum of all
            // on-transformed blocks.
            uint64_t expected_sum = block_size*(block_size-1) / 2;
            uint64_t observed_sum = 0;
            for (uint32_t x = 0; x < plane.block_order.mega_block_width; x++) {
                for (uint32_t y = 0; y < plane.block_order.mega_block_height; y++) {
                    uint32_t offset = get_block_order_offset(x, y, plane.block_order);
                    if (offset > block_size) {
                        auto trans = get_block_order_coords(x, y, plane.block_order);
                        throw std::logic_error("Plane " + to_string(i) + ": x=" + to_string(x) + ", y="
                                               + to_string(y) + " gives transformed coordinates ("
                                               + to_string(trans.first) + ", " + to_string(trans.second) + ") which is larger than the supplied mega_block dimensions. "
                                               + "Remember that the offset must be given in units of blocks." );
                    }
                    observed_sum += offset;
                }
            }

            if (expected_sum != observed_sum) {
                throw std::logic_error("Plane " + to_string(i) + ": block_order is not a bijection. i.e. There are overlapping blocks. This is not supported." );
            }
            i++;
        }
    }

    static bool check_illegal_plane_overlap(const xyuv::format &format) {
        // value: plane_size, has_block_order
        std::map<size_t , std::pair<size_t, bool> > plane_sizes;
        for (auto &plane : format.planes) {
            plane_sizes.insert(
                    std::make_pair(plane.base_offset,
                                   std::make_pair(plane.size,
                                                  plane.block_order.mega_block_height > 1
                                                  || plane.block_order.mega_block_width > 1 )));
        }

        // Map is sorted on first it is enough to check if we cross the boundry of the next.
        auto prev = plane_sizes.begin();
        auto it = prev;
        for (++it; it != plane_sizes.end(); ++it) {
            if (prev->first + prev->second.first > it->first && (it->second.second || prev->second.second)) {
                return false;
            }
        }
        return true;
    }

    // Check whether a line of blocks overflow into the next line.
    static bool check_line_overflow(const xyuv::format &format) {
        // For each channel, determine image size in blocks.
        for (uint32_t i = 0; i < format.channel_blocks.size(); i++) {
            auto & channel_block = format.channel_blocks[i];
            if (channel_block.w == 0 || channel_block.h == 0) {
                continue;
            }
            uint32_t image_w = format.image_w;
            if (i == channel::U || i == channel::V) {
                image_w /= format.chroma_siting.subsampling.macro_px_w;
            }

            uint32_t width_in_blocks = (image_w + channel_block.w -1) / channel_block.w;


            // Only check planes where this channel actually have samples.
            for (auto & sample : channel_block.samples) {
                if ( width_in_blocks * format.planes[sample.plane].block_stride > format.planes[sample.plane].line_stride * 8 ) {
                    return false;
                }
            }
        }
        return true;
    }

    static bool check_plane_overflow(const xyuv::format &format) {
        // For each channel, determine image size in blocks.
        for (uint32_t i = 0; i < format.channel_blocks.size(); i++) {
            auto & channel_block = format.channel_blocks[i];
            if (channel_block.w == 0 || channel_block.h == 0) {
                continue;
            }

            uint32_t image_h = format.image_h;
            if (i == channel::U || i == channel::V) {
                image_h /= format.chroma_siting.subsampling.macro_px_h;
            }

            uint32_t height_in_blocks = (image_h + channel_block.h -1) / channel_block.h;

            // Only check planes where this channel actually have samples.
            for (auto & sample : channel_block.samples) {
                if (height_in_blocks * format.planes[sample.plane].line_stride > format.planes[sample.plane].size ) {
                    return false;
                }
            }

        }
        return true;
    }



    bool validate_format(const xyuv::format &format) {

        if (!check_block_overflow(format)) {
            throw std::logic_error("Block overflow in format.");
        }

        if (!check_sample_overlap(format)) {
            throw std::logic_error("Overlapping samples in format.");
        }

        if (!check_line_overflow(format)) {
            throw std::logic_error("A line of blocks overflow the line stride in format.");
        }

        if (!check_plane_overflow(format)) {
            throw std::logic_error("Plane size to overflow caused by line stride.");
        }

        if (!check_illegal_plane_overlap(format)) {
            throw std::logic_error("The format has overlapping planes combined with block reordering. "
                                           "This is not supported as it does not make sense, try to change "
                                           "the format_template to not use overlapping planes "
                                           "(not always possible).");
        }

        check_block_order(format);

        return true;
    }

}