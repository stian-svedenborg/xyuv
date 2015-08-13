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
#include "file_header.h"
#include "xyuv/structures/format.h"

namespace xyuv {

void to_io_file_header(io_file_header *file_header, const xyuv::format &format);

void to_io_frame_header(io_frame_header *frame_header, const xyuv::format &format);

void to_io_plane_descriptor(io_plane_descriptor *plane_descriptor, const xyuv::plane &plane);

void to_io_channel_block(io_channel_block *channel_block_out, const xyuv::channel_block &channel_block_in);

void to_io_sample_descriptor(io_sample_descriptor *sample_out, const xyuv::sample sample_in);

void from_io_file_header(
        format *fmt,
        uint16_t *offset_to_data,
        uint32_t *checksum,
        const io_file_header &file_header
);

void from_io_frame_header(
        format *fmt,
        uint8_t *n_planes,
        const io_frame_header &frame_header
);


void from_io_plane_descriptor(plane *plane_out, const io_plane_descriptor &plane_descriptor);

void from_io_channel_block(
        xyuv::channel_block *channel_block_in,
        uint32_t *n_samples,
        const io_channel_block &channel_block_out
);

void from_io_sample_descriptor(sample *sample_out, const io_sample_descriptor &sample_in);

} // namespace xyuv