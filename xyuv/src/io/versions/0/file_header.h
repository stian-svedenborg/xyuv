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

#include <cstdint>
#include "xyuv/structures/format_template.h"

namespace xyuv {

// NB! PACKED_STRUCT is only valid within this file.
#ifdef _MSC_VER
#    define PACKED_STRUCT struct
#    pragma pack( push, 1 ) 
#else
#    define PACKED_STRUCT struct __attribute__ ((packed, aligned(1)))
#endif

extern const uint32_t CURRENT_FILE_FORMAT_VERSION;

    namespace fileformat_version_0 {

        PACKED_STRUCT io_chroma_siting {
            uint8_t macro_px_w;
            uint8_t macro_px_h;
            float u_x;
            float u_y;
            float v_x;
            float v_y;
        };

        PACKED_STRUCT io_conversion_matrix {
            float rgb_to_yuv[9];
            float yuv_to_rgb[9];

            float y_range_min, y_range_max;
            float u_range_min, u_range_max;
            float v_range_min, v_range_max;

            float y_packed_range_min, y_packed_range_max;
            float u_packed_range_min, u_packed_range_max;
            float v_packed_range_min, v_packed_range_max;
        };

        PACKED_STRUCT io_frame_header {
            char fourcc[4];
            uint32_t reserved;
            uint8_t origin;

            // Dimensions of the image in luma-samples.
            uint32_t width;
            uint32_t height;
            uint8_t n_planes;

            // High level conversion info
            io_chroma_siting chroma_siting; // Macro_px dimensions are also read from here.
            io_conversion_matrix conversion_matrix;
        };

// Following the io_frame_header struct comes n_planes io_plane_descriptors

// Placeholder Plane descriptor.
        PACKED_STRUCT io_plane_descriptor {
            uint64_t offset_to_plane;
            uint64_t plane_size;
            uint32_t line_stride;  // The stride of a line in this plane.
            uint32_t block_stride; // The stride of a block in this plane.
            uint8_t interleave_mode;
        };

        PACKED_STRUCT io_channel_block {
            // The block dimensions of this channel
            uint16_t block_w;
            uint16_t block_h;
            uint32_t n_continuation_samples;
        };

// Next follows block_w*block_h + n_continuation_samples, io_sample_descriptors.

// When the same color is sampled multiple times in the same block, they are listed in row-major order, 
// i.e: if block_w = 2, block_h = 2 and the samples are listed in order YYYYUV, then the first sample is Y0
// the second Y1 etc in this order:
//  |  Y0  |  Y1  |
//  |  Y2  |  Y3  |
        PACKED_STRUCT io_sample_descriptor {
            uint8_t plane;
            uint8_t integer_bits;
            uint8_t fractional_bits;
            bool has_continuation;
            uint16_t offset;
        };

#ifdef _MSC_VER
#    pragma pack( pop ) 
#endif

    } // fileformat_version_0

} // namespace xyuv