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
#include <xyuv/yuv_image.h>
#include <xyuv/frame.h>
#include <xyuv/structures/constants.h>

#include "config-parser/minicalc/minicalc.h"
#include "utility.h"
#include "assert.h"
#include "block_reorder.h"

#include <algorithm>
#include <string.h>

namespace xyuv {

using unorm_t = uint64_t;

    static uint32_t get_line(uint32_t block_line, xyuv::interleave_pattern interleave_pattern, uint32_t height_in_blocks) {
        switch (interleave_pattern) {
        case xyuv::interleave_pattern::NO_INTERLEAVING:
            return block_line;
        case xyuv::interleave_pattern::INTERLEAVE_0_2_4__1_3_5:
            if (block_line & 1) {
                uint32_t split_at = (height_in_blocks + 1) / 2;
                return split_at + (block_line/2);
            }
            else {
                return (block_line/2);
            }
        case xyuv::interleave_pattern::INTERLEAVE_1_3_5__0_2_4:
            if (block_line & 1) {
                return (block_line/2);
            }
            else {
                uint32_t split_at = (height_in_blocks) / 2;
                return split_at + (block_line/2);
            }
        }
        XYUV_ASSERT(false && "Unreachable");
    }


static inline unorm_t round_to_unorm(double val) {
    using std::floor;
    return static_cast<unorm_t>(floor(val + 0.5));
}

static unorm_t to_unorm(float value, uint8_t integer_bits, uint8_t fractional_bits,
                        const std::pair<float, float> &range) {
    using std::min;
    XYUV_ASSERT_RANGE(0.0f, 1.0f, value);
    XYUV_ASSERT(integer_bits + fractional_bits <= 8 * sizeof(unorm_t));
    XYUV_ASSERT(integer_bits < 8 * sizeof(unorm_t));

    // What is the largest integer value representable
    unorm_t max = (0x1ull << integer_bits) - 1;
    max <<= fractional_bits;

    double dbl_val = value;

    // Scale value to range boundaries:
    dbl_val *= (range.second - range.first);
    dbl_val += range.first;

    // Convert to unorm
    unorm_t bits = round_to_unorm(dbl_val * max);

    return bits;
}

static float from_unorm(unorm_t unorm, uint8_t integer_bits, uint8_t fractional_bits,
                        const std::pair<float, float> &range) {
    XYUV_ASSERT(integer_bits + fractional_bits <= 8 * sizeof(unorm_t));
    XYUV_ASSERT(integer_bits < 8 * sizeof(unorm_t));

    unorm_t max = (0x1ull << integer_bits) - 1;
    max <<= fractional_bits;

    // Convert to double
    double dbl_val = static_cast<double>(unorm) / max;

    // Convert back to [0,1]
    dbl_val -= range.first;
    dbl_val /= (range.second - range.first);

    return clamp(0.0f, 1.0f, static_cast<float>(dbl_val));
}



//! \brief Buffer is seen as a continuous stream of bits from lsb of LSB to msb of MSB.
//! Offset is in bits from least significant bit of buffer to least significant bit of value
//! Value is sent by reference because it needs to be destroyed when writing continuation blocks.
void write_bits(uint8_t *buffer, uint64_t offset, uint8_t bits, unorm_t &value) {

    for (uint8_t i = 0; i < bits; i++) {
        set_bit(buffer, offset + i, (value & 0x1) != 0);
        value >>= 1;
    }
}

//! \brief Buffer is seen as a continuous stream of bits from lsb of LSB to msb of MSB.
//! Offset is in bits from least significant bit of buffer to least significant bit of value
unorm_t read_bits(unorm_t & value, const uint8_t *buffer, uint64_t offset, uint8_t bits) {

    for (uint8_t i = bits-1; i < bits; i--) {
        value <<= 1;
        if (get_bit(buffer, offset + i)) {
            value |= 0x1;
        }
    }
    return value;
}

// The block iterator will iterate over each pixel in a block before moving on to the next one.
class block_iterator {
public:
    block_iterator(uint16_t block_w, uint16_t block_h, surface<float> *surf) :
            block_w(block_w), block_h(block_h),
            block_x(0), block_y(0),
            x(0), y(0),
            surf(surf) { }

    block_iterator(uint16_t block_w, uint16_t block_h, const surface<float> *surf) :
            block_w(block_w), block_h(block_h),
            block_x(0), block_y(0),
            x(0), y(0),
            surf(const_cast<surface<float> *>(surf)) { }

    float *advance() {
        float *ret = &(surf->at(block_x + x, block_y + y));
        if (++x == block_w) {
            x = 0;
            if (++y == block_h) {
                y = 0;
                if ((block_x += block_w) >= surf->width()) {
                    block_x = 0;
                    block_y += block_h;
                }
            }
        }
        return ret;
    }

private:
    uint32_t block_w, block_h;
    uint32_t block_x, block_y;
    uint32_t x, y;
    surface<float> *surf;
};



static void encode_channel(uint8_t *base_addr, const channel_block &block, const surface<float> &surf,
                           const std::vector<plane> &planes, const std::pair<float, float> range, bool negative_line_stride) {
    block_iterator it(block.w, block.h, &surf);

    uint32_t n_blocks_in_line = (surf.width() / block.w);
    uint32_t n_block_lines = (surf.height() / block.h);

    // We need to support negative line stride.
    std::vector<std::pair<std::ptrdiff_t , std::ptrdiff_t >> line_offsets;
    for (const auto & plane : planes ) {
        if (negative_line_stride) {
            std::ptrdiff_t line_stride = -static_cast<std::ptrdiff_t>(plane.line_stride);
            // Each line  is still left to right.
            line_offsets.emplace_back(line_stride, plane.size + line_stride);
        } else {
            line_offsets.emplace_back(plane.line_stride, 0);
        }
    }

    // We need to preprocess the sample array to support continuation samples.
    std::vector<sample> samples;
    for (std::size_t i = 0; i < block.samples.size();) {
        const xyuv::sample &sample = block.samples[i];
        if (!sample.has_continuation) {
            samples.push_back(sample);
            ++i;
        }
        else {
            // Create a descriptor block containing all the bits of the samples.
            xyuv::sample sample_descriptor;
            sample_descriptor.integer_bits = 0;
            sample_descriptor.fractional_bits = 0;
            sample_descriptor.has_continuation = true;
            samples.push_back(sample_descriptor);
            size_t descriptor_pos = samples.size() -1;

            // Create a stack of all the bits in the sample.
            // We need to reverse the order of the samples to encode them correctly.
            std::vector<const xyuv::sample*> bit_stack;
            do {
                // Update descriptor
                samples[descriptor_pos].integer_bits += block.samples[i].integer_bits;
                samples[descriptor_pos].fractional_bits += block.samples[i].fractional_bits;
                bit_stack.push_back(&(block.samples[i]));
            } while(block.samples[i++].has_continuation);

            for (auto rit = bit_stack.rbegin(); rit != bit_stack.rend(); ++rit) {
                samples.push_back(*(*rit));
                samples.back().has_continuation = true;
            }

            samples.back().has_continuation = false;
        }
    }

    // Finally iterate over the image
    for (uint32_t line = 0; line < n_block_lines; line++) {
        // Precompute interleaved lines
        uint32_t interleaved_line[3] = {
                get_line(line, static_cast<interleave_pattern>(0), n_block_lines),
                get_line(line, static_cast<interleave_pattern>(1), n_block_lines),
                get_line(line, static_cast<interleave_pattern>(2), n_block_lines),
        };
        for (uint32_t b = 0; b < n_blocks_in_line; b++) {
            for (std::size_t s = 0; s < samples.size(); ) {
                uint8_t integer_bits = samples[s].integer_bits;
                uint8_t fractional_bits = samples[s].fractional_bits;

                float value = *it.advance();
                unorm_t unorm = to_unorm(value, integer_bits, fractional_bits, range);

                // If we hit a continuation block here, it means that we have the
                // Total bits descriptor and should skip it for the purpose of actual storing.
                if (samples[s].has_continuation) {
                    s++;
                }

                do {
                    const xyuv::sample &sample = samples[s];

                    uint8_t * ptr_to_line =
                            // Start with offset to frame
                            base_addr +
                            // Add offset to lowest byte in plane.
                            planes[sample.plane].base_offset +
                            // Add the size of the plane if applicable.
                            line_offsets[sample.plane].second +
                            // Add offset to current line.
                            interleaved_line[static_cast<uint32_t>(planes[sample.plane].interleave_mode)] * line_offsets[sample.plane].first;

                    // Read bits written bits from LSb fractional to MSb integer bit.
                    write_bits( ptr_to_line,
                               b * planes[sample.plane].block_stride + sample.offset,
                               sample.integer_bits + sample.fractional_bits, unorm);

                } while (samples[s++].has_continuation);
            }
        }
    }
}

static xyuv::frame internal_encode_frame(const yuv_image &yuva_in, const xyuv::format &format) {

    // Determine the byte size of each plane.
    bool has_y = !format.channel_blocks[channel::Y].samples.empty();
    bool has_u = !format.channel_blocks[channel::U].samples.empty();
    bool has_v = !format.channel_blocks[channel::V].samples.empty();
    bool has_a = !format.channel_blocks[channel::A].samples.empty();

    std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[format.size]);
    // Fill buffer with poison values to make padding "undefined" yet deterministic.
    poison_buffer(buffer.get(), format.size);

    bool has_negative_line_stride = (format.origin == image_origin::LOWER_LEFT);

    if (has_y)
        encode_channel(
                buffer.get(),
                format.channel_blocks[channel::Y],
                yuva_in.y_plane,
                format.planes,
                format.conversion_matrix.y_packed_range,
                has_negative_line_stride
        );
    if (has_u)
        encode_channel(
                buffer.get(),
                format.channel_blocks[channel::U],
                yuva_in.u_plane,
                format.planes,
                format.conversion_matrix.u_packed_range,
                has_negative_line_stride
        );
    if (has_v)
        encode_channel(
                buffer.get(),
                format.channel_blocks[channel::V],
                yuva_in.v_plane,
                format.planes,
                format.conversion_matrix.v_packed_range,
                has_negative_line_stride
        );

    if (has_a) {
        const surface<pixel_quantum> *surf = &(yuva_in.a_plane);

        // Alpha is special, if it is not present, we need to
        // create a surface and default it to one.
        std::unique_ptr<surface<pixel_quantum>> tempsurf;
        if (yuva_in.a_plane.empty()) {
            tempsurf.reset(new surface<pixel_quantum>(yuva_in.image_w, yuva_in.image_h));
            tempsurf->fill(1.0f);
            surf = tempsurf.get();
        }

        encode_channel(
                buffer.get(),
                format.channel_blocks[channel::A],
                *surf,
                format.planes,
                std::make_pair<float, float>(0.0f, 1.0f),
                has_negative_line_stride
        );


    }

    for (auto & plane : format.planes ) {
        reorder_transform(buffer.get(), plane );
    }

    // Init frame info.
    xyuv::frame frame;
    frame.data = std::move(buffer);
    frame.format = format;

    return frame;
}

static void decode_channel(const uint8_t *base_addr, const channel_block &block, surface<float> *surf,
                           const std::vector<plane> &planes, const std::pair<float, float> range, bool negative_line_stride) {
    block_iterator it(block.w, block.h, surf);

    uint32_t n_blocks_in_line = (surf->width() / block.w);
    uint32_t n_block_lines = (surf->height() / block.h);

    // We need to support negative line stride.
    std::vector<std::pair<std::ptrdiff_t , std::ptrdiff_t >> line_offsets;
    for (const auto & plane : planes ) {
        if (negative_line_stride) {
            std::ptrdiff_t line_stride = -static_cast<std::ptrdiff_t>(plane.line_stride);
            // Each line  is still left to right.
            line_offsets.emplace_back(line_stride, plane.size + line_stride);
        } else {
            line_offsets.emplace_back(plane.line_stride, 0);
        }
    }

    // We need to preprocess the sample array to support continuation samples.
    std::vector<sample> samples;
    for (std::size_t i = 0; i < block.samples.size(); ) {
        const xyuv::sample &sample = block.samples[i];
        if (!sample.has_continuation) {
            samples.push_back(sample);
            i++;
        }
        else {
            // Create a descriptor block containing all the bits of the samples.
            xyuv::sample sample_descriptor;
            sample_descriptor.integer_bits = 0;
            sample_descriptor.fractional_bits = 0;
            sample_descriptor.has_continuation = true;
            samples.push_back(sample_descriptor);
            size_t descriptor_pos = samples.size() -1;
            do {
                // Update descriptor
                samples[descriptor_pos].integer_bits += block.samples[i].integer_bits;
                samples[descriptor_pos].fractional_bits += block.samples[i].fractional_bits;
                samples.push_back(block.samples[i]);
            } while(block.samples[i++].has_continuation);
        }
    }

    for (uint32_t line = 0; line < n_block_lines; line++) {
        for (uint32_t b = 0; b < n_blocks_in_line; b++) {
            for (std::size_t s = 0; s < samples.size(); ) {


                uint8_t integer_bits = samples[s].integer_bits;
                uint8_t fractional_bits = samples[s].fractional_bits;


                // If we hit a continuation block here, it means that we have the
                // Total bits descriptor and should skip it for the purpose of actual loading.
                if (samples[s].has_continuation) {
                    s++;
                }

                unorm_t unorm = 0;
                do {
                    const xyuv::sample &sample = samples[s];

                    const uint8_t * ptr_to_line =
                            // Start with offset to frame
                            base_addr +
                            // Add offset to lowest byte in plane.
                            planes[sample.plane].base_offset +
                            // Add the size of the plane if applicable.
                            line_offsets[sample.plane].second +
                            // Add offset to current line.
                            get_line(line, planes[sample.plane].interleave_mode, n_block_lines) * line_offsets[sample.plane].first;

                    // Read bits reads bits from MSb integer to LSb fractional bit.
                    read_bits( unorm,
                               ptr_to_line,
                               b * planes[sample.plane].block_stride + sample.offset,
                               sample.integer_bits + sample.fractional_bits);

                } while (samples[s++].has_continuation);

                float *value = it.advance();
                *value = from_unorm(unorm, integer_bits, fractional_bits, range);
            }
        }
    }
}


yuv_image decode_frame(const xyuv::frame &frame_in) {

    // Determine the size of each plane.
    bool has_y = !frame_in.format.channel_blocks[channel::Y].samples.empty();
    bool has_u = !frame_in.format.channel_blocks[channel::U].samples.empty();
    bool has_v = !frame_in.format.channel_blocks[channel::V].samples.empty();
    bool has_a = !frame_in.format.channel_blocks[channel::A].samples.empty();

    yuv_image yuva_out = create_yuv_image(
            frame_in.format.image_w,
            frame_in.format.image_h,
            frame_in.format.chroma_siting,
            has_y,
            has_u,
            has_v,
            has_a
    );

    bool has_negative_line_stride = (frame_in.format.origin == image_origin::LOWER_LEFT);

    const uint8_t * raw_data = frame_in.data.get();

    std::unique_ptr<uint8_t[]> tmp_buffer;
    if (needs_reorder(frame_in.format)) {
        // Todo: If needed optimize this for memory.
        // At some point we will have allocated 2x frame + 1 plane.
        tmp_buffer.reset(new uint8_t[frame_in.format.size]);
        memcpy(tmp_buffer.get(), raw_data, frame_in.format.size);

        // Use the copy instead.
        raw_data = tmp_buffer.get();

        for (auto & plane : frame_in.format.planes) {
            reorder_inverse(tmp_buffer.get(), plane);
        }
    }

    if (has_y)
        decode_channel(
                raw_data,
                frame_in.format.channel_blocks[channel::Y],
                &(yuva_out.y_plane),
                frame_in.format.planes,
                frame_in.format.conversion_matrix.y_packed_range,
                has_negative_line_stride
        );
    if (has_u)
        decode_channel(
                raw_data,
                frame_in.format.channel_blocks[channel::U],
                &(yuva_out.u_plane),
                frame_in.format.planes,
                frame_in.format.conversion_matrix.u_packed_range,
                has_negative_line_stride
        );
    if (has_v)
        decode_channel(
                raw_data,
                frame_in.format.channel_blocks[channel::V],
                &(yuva_out.v_plane),
                frame_in.format.planes,
                frame_in.format.conversion_matrix.v_packed_range,
                has_negative_line_stride
        );
    if (has_a)
        decode_channel(
                raw_data,
                frame_in.format.channel_blocks[channel::A],
                &(yuva_out.a_plane),
                frame_in.format.planes,
                std::make_pair<float, float>(0.0f, 1.0f),
                has_negative_line_stride
        );



    return yuva_out;
}

// This is the entry_point for the encode function, and will make sure the result
// sane.
xyuv::frame encode_frame(const xyuv::yuv_image &yuva_in, const xyuv::format &format) {
    bool dimensions_match = yuva_in.image_w == format.image_w && yuva_in.image_h == format.image_h;

    // Short path.
    if (dimensions_match && yuva_in.siting == format.chroma_siting) {
        return internal_encode_frame(yuva_in, format);
    }

    // Otherwise we will need to do some conversion.
    const yuv_image *image = &yuva_in;
    yuv_image temp;

    if (!dimensions_match) {
        if (!is_444(image->siting.subsampling)) {
            temp = up_sample(*image);
            image = &temp;
        }
        temp = scale_yuv_image(*image, format.image_w, format.image_h);
        image = &temp;
    }

    if (!(image->siting == format.chroma_siting)) {
        if (image->siting.subsampling.macro_px_w > 1 ||
            image->siting.subsampling.macro_px_h > 1) {
            temp = up_sample(*image);
            image = &temp;
        }

        // At this point *image is 444
        if (format.chroma_siting.subsampling.macro_px_w > 1 ||
            format.chroma_siting.subsampling.macro_px_h > 1) {
            temp = down_sample(*image, format.chroma_siting);
            image = &temp;
        }
    }

    return internal_encode_frame(*image, format);
}

} // namespace xyuv
