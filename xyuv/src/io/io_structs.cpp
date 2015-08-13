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

#include "io_structs.h"

#include <cstring>
#include "../assert.h"
#include "endianess.h"

namespace xyuv {

static void to_io_chroma_siting(io_chroma_siting *chroma_siting_out, const xyuv::chroma_siting &chroma_siting_in) {
    chroma_siting_out->macro_px_w = host_to_be(chroma_siting_in.subsampling.macro_px_w);
    chroma_siting_out->macro_px_h = host_to_be(chroma_siting_in.subsampling.macro_px_h);

    // IEEE floats, don't need to change endianess. (Hopefully)
    chroma_siting_out->u_x = chroma_siting_in.u_sample_point.first;
    chroma_siting_out->u_y = chroma_siting_in.u_sample_point.second;

    chroma_siting_out->u_x = chroma_siting_in.v_sample_point.first;
    chroma_siting_out->u_y = chroma_siting_in.v_sample_point.second;
}

static void to_io_conversion_matrix(io_conversion_matrix *conversion_matrix_out,
                                    const xyuv::conversion_matrix &conversion_matrix_in) {
    memcpy(conversion_matrix_out->rgb_to_yuv, conversion_matrix_in.rgb_to_yuv, sizeof(conversion_matrix_in.rgb_to_yuv));
    memcpy(conversion_matrix_out->yuv_to_rgb, conversion_matrix_in.yuv_to_rgb, sizeof(conversion_matrix_in.yuv_to_rgb));

    conversion_matrix_out->y_range_min = conversion_matrix_in.y_range.first;
    conversion_matrix_out->y_range_max = conversion_matrix_in.y_range.second;
    conversion_matrix_out->u_range_min = conversion_matrix_in.u_range.first;
    conversion_matrix_out->u_range_max = conversion_matrix_in.u_range.second;
    conversion_matrix_out->v_range_min = conversion_matrix_in.v_range.first;
    conversion_matrix_out->v_range_max = conversion_matrix_in.v_range.second;

    conversion_matrix_out->y_packed_range_min = conversion_matrix_in.y_packed_range.first;
    conversion_matrix_out->y_packed_range_max = conversion_matrix_in.y_packed_range.second;
    conversion_matrix_out->u_packed_range_min = conversion_matrix_in.u_packed_range.first;
    conversion_matrix_out->u_packed_range_max = conversion_matrix_in.u_packed_range.second;
    conversion_matrix_out->v_packed_range_min = conversion_matrix_in.v_packed_range.first;
    conversion_matrix_out->v_packed_range_max = conversion_matrix_in.v_packed_range.second;
}

static std::pair<uint16_t, uint32_t> calculate_header_properties(const xyuv::format &format) {
    uint16_t header_size = 0;

    header_size += sizeof(io_file_header);
    header_size += sizeof(io_frame_header);
    header_size += sizeof(io_plane_descriptor) * format.planes.size();

    for (auto &block : format.channel_blocks) {
        header_size += sizeof(io_channel_block);
        header_size += sizeof(io_sample_descriptor) + block.samples.size();
    }

    return std::make_pair(header_size, 0u);
}

void to_io_file_header(io_file_header *file_header, const xyuv::format &format) {
    memcpy(file_header->magic, "XYUV_FMT", 8);

    file_header->version = host_to_be(CURRENT_FILE_FORMAT_VERSION);
    file_header->payload_size = host_to_be(format.size);

    std::pair<uint16_t, uint32_t> offset_and_checksum = calculate_header_properties(format);
    file_header->offset_to_data = host_to_be(offset_and_checksum.first);
    file_header->checksum = host_to_be(offset_and_checksum.second);
}

void to_io_frame_header(io_frame_header *frame_header, const xyuv::format &format) {
    frame_header->width = host_to_be(format.image_w);
    frame_header->height = host_to_be(format.image_h);
    frame_header->n_planes = host_to_be(static_cast<uint8_t >(format.planes.size()));
    frame_header->origin = host_to_be(static_cast<uint8_t>(format.origin));

    to_io_chroma_siting(&frame_header->chroma_siting, format.chroma_siting);
    to_io_conversion_matrix(&frame_header->conversion_matrix, format.conversion_matrix);
}

void to_io_plane_descriptor(io_plane_descriptor *plane_descriptor, const xyuv::plane &plane) {
    plane_descriptor->offset_to_plane = host_to_be(plane.base_offset);
    plane_descriptor->plane_size = host_to_be(plane.size);
    plane_descriptor->line_stride = host_to_be(plane.line_stride);
    plane_descriptor->block_stride = host_to_be(plane.block_stride);

    plane_descriptor->interleave_mode = host_to_be(static_cast<uint8_t>(plane.interleave_mode));
}

void to_io_channel_block(io_channel_block *channel_block_out, const xyuv::channel_block &channel_block_in) {
    channel_block_out->block_w = host_to_be(channel_block_in.w);
    channel_block_out->block_h = host_to_be(channel_block_in.h);
    channel_block_out->n_continuation_samples =
            host_to_be(
                    static_cast<uint32_t>(
                            channel_block_in.samples.size() - (channel_block_in.w * channel_block_in.h)
                    )
            );
}

void to_io_sample_descriptor(io_sample_descriptor *sample_out, const xyuv::sample sample_in) {
    sample_out->plane = host_to_be(sample_in.plane);
    sample_out->integer_bits = host_to_be(sample_in.integer_bits);
    sample_out->fractional_bits = host_to_be(sample_in.fractional_bits);
    sample_out->has_continuation = host_to_be(sample_in.has_continuation);
    sample_out->offset = host_to_be(sample_in.offset);
}

/* ================================================
 *
 * ================================================
 */
static void from_io_chroma_siting(chroma_siting *chroma_siting_out, const io_chroma_siting &chroma_siting_in) {
    chroma_siting_out->subsampling.macro_px_w = be_to_host(chroma_siting_in.macro_px_w);
    chroma_siting_out->subsampling.macro_px_h = be_to_host(chroma_siting_in.macro_px_h);

    // IEEE floats, don't need to change endianess. (Hopefully)
    chroma_siting_out->u_sample_point.first = chroma_siting_in.u_x;
    chroma_siting_out->u_sample_point.second = chroma_siting_in.u_y;

    chroma_siting_out->v_sample_point.first = chroma_siting_in.v_x;
    chroma_siting_out->v_sample_point.second = chroma_siting_in.v_y;
}

static void from_io_conversion_matrix(conversion_matrix *conversion_matrix_out,
                                      const io_conversion_matrix &conversion_matrix_in) {
    memcpy(conversion_matrix_out->rgb_to_yuv, conversion_matrix_in.rgb_to_yuv, sizeof(conversion_matrix_in.rgb_to_yuv));
    memcpy(conversion_matrix_out->yuv_to_rgb, conversion_matrix_in.yuv_to_rgb, sizeof(conversion_matrix_in.yuv_to_rgb));

    conversion_matrix_out->y_range.first = conversion_matrix_in.y_range_min;
    conversion_matrix_out->y_range.second = conversion_matrix_in.y_range_max;
    conversion_matrix_out->u_range.first = conversion_matrix_in.u_range_min;
    conversion_matrix_out->u_range.second = conversion_matrix_in.u_range_max;
    conversion_matrix_out->v_range.first = conversion_matrix_in.v_range_min;
    conversion_matrix_out->v_range.second = conversion_matrix_in.v_range_max;

    conversion_matrix_out->y_packed_range.first = conversion_matrix_in.y_packed_range_min;
    conversion_matrix_out->y_packed_range.second = conversion_matrix_in.y_packed_range_max;
    conversion_matrix_out->u_packed_range.first = conversion_matrix_in.u_packed_range_min;
    conversion_matrix_out->u_packed_range.second = conversion_matrix_in.u_packed_range_max;
    conversion_matrix_out->v_packed_range.first = conversion_matrix_in.v_packed_range_min;
    conversion_matrix_out->v_packed_range.second = conversion_matrix_in.v_packed_range_max;

}

void from_io_file_header(
        xyuv::format *format,
        uint16_t *offset_to_data,
        uint32_t *checksum,
        const io_file_header &file_header
) {
    XYUV_ASSERT(strncmp(file_header.magic, "XYUV_FMT", 8) == 0);
    XYUV_ASSERT(file_header.version == CURRENT_FILE_FORMAT_VERSION);

    format->size = be_to_host(file_header.payload_size);
    *offset_to_data = be_to_host(file_header.offset_to_data);
    *checksum = be_to_host(file_header.checksum);
}

void from_io_frame_header(
        xyuv::format *format,
        uint8_t *n_planes,
        const io_frame_header &frame_header
) {
    format->image_w = be_to_host(frame_header.width);
    format->image_h = be_to_host(frame_header.height);
    format->origin = static_cast<image_origin>(be_to_host(frame_header.origin));

    from_io_chroma_siting(&format->chroma_siting, frame_header.chroma_siting);
    from_io_conversion_matrix(&format->conversion_matrix, frame_header.conversion_matrix);

    *n_planes = be_to_host(frame_header.n_planes);
}

void from_io_plane_descriptor(plane *plane, const io_plane_descriptor &plane_descriptor) {
    plane->base_offset = be_to_host(plane_descriptor.offset_to_plane);
    plane->size = be_to_host(plane_descriptor.plane_size);
    plane->line_stride = be_to_host(plane_descriptor.line_stride);
    plane->block_stride = be_to_host(plane_descriptor.block_stride);

    plane->interleave_mode = static_cast<interleave_pattern>(be_to_host(plane_descriptor.interleave_mode));
}

void from_io_channel_block(
        xyuv::channel_block *channel_block_out,
        uint32_t *n_samples,
        const io_channel_block &channel_block_in
) {

    channel_block_out->w = be_to_host(channel_block_in.block_w);
    channel_block_out->h = be_to_host(channel_block_in.block_h);

    *n_samples = channel_block_out->w * channel_block_out->h + be_to_host(channel_block_in.n_continuation_samples);
}

void from_io_sample_descriptor(sample *sample_out, const io_sample_descriptor &sample_in) {
    sample_out->plane = be_to_host(sample_in.plane);
    sample_out->integer_bits = be_to_host(sample_in.integer_bits);
    sample_out->fractional_bits = be_to_host(sample_in.fractional_bits);
    sample_out->has_continuation = be_to_host(sample_in.has_continuation);
    sample_out->offset = be_to_host(sample_in.offset);
}

} // namespace xyuv