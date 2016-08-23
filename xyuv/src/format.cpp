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

#include <xyuv.h>
#include <xyuv/structures/format.h>
#include <xyuv/structures/format_template.h>
#include <xyuv/minicalc.h>
#include <xyuv/frame.h>
#include "to_string.h"
#include "assert.h"
#include "config-parser/format_validator.h"
#include "utility.h"

#include <algorithm>
#include <cstring>

namespace xyuv {

xyuv::format create_format(
        uint32_t width,
        uint32_t height,
        const xyuv::format_template &format_template,
        const xyuv::conversion_matrix &conversion_matrix,
        const xyuv::chroma_siting &chroma_siting
) {
    // First do some simple sanity checking:
    XYUV_ASSERT(
            format_template.subsampling.macro_px_w == chroma_siting.subsampling.macro_px_w
            && format_template.subsampling.macro_px_h == chroma_siting.subsampling.macro_px_h
    );

    xyuv::format format;

    format.size = 0;

    // Start out with the plane descriptor as it is the only one that might fail.
    // The expressions have certain variable available. The rules are as follows:
    // - All expression have the following variables available:
    //      - image_w, image_h, macro_px_w, macro_px_h
    //      - plane[i].line_stride, plane[i].base_offset, plane[i].size of all previous planes.
    // - The plane_size expression has the line_stride of the same plane available.

    std::unordered_map<std::string, uint64_t> available_variables = {
            // These are the global variables.
            {"image_w", width},
            {"image_h", height},
            {"macro_px_w", format_template.subsampling.macro_px_w},
            {"macro_px_h", format_template.subsampling.macro_px_h}
    };

    for (std::size_t i = 0; i < format_template.planes.size(); i++) {
        xyuv::plane plane;

        // If the texel has mega_blocks, ensure we allocate enough padding
        uint32_t padded_width  = next_multiple(width, format_template.planes[i].block_order.mega_block_width);
        uint32_t padded_height = next_multiple(height, format_template.planes[i].block_order.mega_block_height);

        available_variables["image_w"] = padded_width;
        available_variables["image_h"] = padded_height;

        // Set default.
        plane.base_offset = minicalc_evaluate(format_template.planes[i].base_offset_expression, &available_variables);
        plane.line_stride = static_cast<uint32_t>(minicalc_evaluate(format_template.planes[i].line_stride_expression,
                                                                    &available_variables));

        // Now do the plane size.
        available_variables["line_stride"] = plane.line_stride;
        plane.size = minicalc_evaluate(format_template.planes[i].plane_size_expression, &available_variables);
        available_variables.erase("line_stride");

        // Set remaining fields.
        plane.block_stride = format_template.planes[i].block_stride;
        plane.interleave_mode = format_template.planes[i].interleave_mode;
        plane.block_order = format_template.planes[i].block_order;

        // Update variables with the new fields.
        std::string prefix = "plane[" + to_string(i) + "].";
        available_variables[prefix + "base_offset"] = plane.base_offset;
        available_variables[prefix + "line_stride"] = plane.line_stride;
        available_variables[prefix + "plane_size"] = plane.size;

        format.planes.push_back(plane);

        // Update format footprint.
        // We are not guaranteed that user supply the planes in ascending memory-layout order.
        // So we must pick the largest offset + size combination.
        format.size = std::max(format.size, plane.base_offset + plane.size);
    }

    // Copy remaining fields into format.
    format.fourcc = format_template.fourcc;
    format.image_w = width;
    format.image_h = height;
    format.origin = format_template.origin;
    format.chroma_siting = chroma_siting;
    format.conversion_matrix = conversion_matrix;

    for (std::size_t i = 0; i < format.channel_blocks.size(); i++) {
        format.channel_blocks[i] = format_template.channel_blocks[i];
    }

    // Ensure that the format we created is conformant.
    validate_format(format);

    return format;
}

xyuv::frame create_frame(
        const xyuv::format &format,
        const uint8_t *raw_data,
        uint64_t raw_data_size
) {
    // First, allocate the buffer.
    std::unique_ptr<uint8_t[]> data(new uint8_t[format.size]);

    // If raw_data has been supplied. Copy it.
    if (raw_data != nullptr) {
        memmove(data.get(), raw_data, std::min(format.size, raw_data_size));
    }

    // Assign and return.
    xyuv::frame frame;
    frame.format = format;
    frame.data = std::move(data);

    return frame;
};

} // namespace xyuv