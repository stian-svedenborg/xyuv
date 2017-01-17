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

#include <xyuv/structures/format.h>
#include <xyuv/format_json.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <map>
#include "../to_string.h"
#include "../assert.h"

namespace xyuv {

    using Allocator = rapidjson::Document::AllocatorType;

    static rapidjson::Value set_block_order(Allocator & allocator, const ::block_order & block_order_in) {
        rapidjson::Value block_order_out(rapidjson::kObjectType);

        block_order_out.AddMember("mega_block_width", block_order_in.mega_block_width, allocator);
        block_order_out.AddMember("mega_block_height", block_order_in.mega_block_height, allocator);

        rapidjson::Value x_mask(rapidjson::kArrayType);
        rapidjson::Value y_mask(rapidjson::kArrayType);

        for (auto v : block_order_in.x_mask) {
            std::string string_val = v >= ::block_order::NOT_USED ? std::string("-") : xyuv::to_string(v);
            x_mask.PushBack( rapidjson::Value(string_val.c_str(), allocator).Move(), allocator);
        }

        for (auto v : block_order_in.y_mask) {
            std::string string_val = v >= ::block_order::NOT_USED ? std::string("-") : xyuv::to_string(v);
            y_mask.PushBack( rapidjson::Value(string_val.c_str(), allocator).Move(), allocator);
        }

        block_order_out.AddMember("x_mask", x_mask, allocator);
        block_order_out.AddMember("y_mask", y_mask, allocator);

        return block_order_out;
    }

    static rapidjson::Value set_plane(Allocator & allocator, const xyuv::plane & plane_in) {

        static const std::map<xyuv::interleave_pattern, const char *> interleave_mapping = {
                {xyuv::interleave_pattern::NO_INTERLEAVING, "NO_INTERLEAVING"},
                {xyuv::interleave_pattern::INTERLEAVE_0_2_4__1_3_5, "INTERLEAVE_0_2_4__1_3_5"},
                {xyuv::interleave_pattern::INTERLEAVE_1_3_5__0_2_4, "INTERLEAVE_1_3_5__0_2_4"},
        };

        rapidjson::Value plane_out(rapidjson::kObjectType);

        plane_out.AddMember("base_offset", plane_in.base_offset, allocator);
        plane_out.AddMember("size", plane_in.size, allocator);
        plane_out.AddMember("line_stride", plane_in.line_stride, allocator);
        plane_out.AddMember("block_stride", plane_in.block_stride, allocator);

        rapidjson::Value interleave_mode(interleave_mapping.find(plane_in.interleave_mode)->second, allocator);
        plane_out.AddMember("interleave_mode", interleave_mode, allocator);

        plane_out.AddMember("block_order", set_block_order(allocator, plane_in.block_order).Move(), allocator);

        return plane_out;
    }

    static rapidjson::Value set_planes(Allocator & allocator, const std::vector<xyuv::plane> & planes_in) {
        rapidjson::Value planes(rapidjson::kArrayType);

        for ( const auto & plane : planes_in) {
            planes.PushBack(set_plane(allocator, plane).Move(), allocator);
        }

        return planes;
    }

    static rapidjson::Value set_subsampling(Allocator& allocator, const xyuv::subsampling & subsampling_in) {
        rapidjson::Value subsampling_out(rapidjson::kObjectType);

        subsampling_out.AddMember("macro_px_w", static_cast<uint32_t>(subsampling_in.macro_px_w), allocator);
        subsampling_out.AddMember("macro_px_h", static_cast<uint32_t>(subsampling_in.macro_px_h), allocator);

        return subsampling_out;
    }

    static rapidjson::Value set_sampling_point(Allocator& allocator, const std::pair<float, float> & sampling_point_in ) {
        rapidjson::Value sampling_point_out(rapidjson::kObjectType);

        sampling_point_out.AddMember("x", sampling_point_in.first, allocator);
        sampling_point_out.AddMember("y", sampling_point_in.second, allocator);

        return sampling_point_out;

    }

    static rapidjson::Value set_chroma_siting(Allocator& allocator, const xyuv::chroma_siting & chroma_siting_in) {
        rapidjson::Value chroma_siting_out(rapidjson::kObjectType);

        chroma_siting_out.AddMember("subsampling", set_subsampling(allocator, chroma_siting_in.subsampling).Move(), allocator);
        chroma_siting_out.AddMember("u_sample_point", set_sampling_point(allocator, chroma_siting_in.u_sample_point).Move(), allocator);
        chroma_siting_out.AddMember("v_sample_point", set_sampling_point(allocator, chroma_siting_in.v_sample_point).Move(), allocator);

        return chroma_siting_out;
    }

    static rapidjson::Value set_3x3_matrix(Allocator & allocator, const float (&matrix_in)[9]) {
        rapidjson::Value matrix_out(rapidjson::kArrayType);

        for (auto v : matrix_in) {
            matrix_out.PushBack(v, allocator);
        }

        return matrix_out;
    }

    static rapidjson::Value set_range(Allocator& allocator, const std::pair<float, float> & range ) {
        rapidjson::Value sampling_point_out(rapidjson::kObjectType);

        sampling_point_out.AddMember("min", range.first, allocator);
        sampling_point_out.AddMember("max", range.second, allocator);

        return sampling_point_out;
    }

    static rapidjson::Value set_conversion_matrix(Allocator & allocator, const xyuv::conversion_matrix & conversion_matrix_in) {
        rapidjson::Value conversion_matrix_out(rapidjson::kObjectType);

        conversion_matrix_out.AddMember("rgb_to_yuv", set_3x3_matrix(allocator, conversion_matrix_in.rgb_to_yuv).Move(), allocator);
        conversion_matrix_out.AddMember("yuv_to_rgb", set_3x3_matrix(allocator, conversion_matrix_in.yuv_to_rgb).Move(), allocator);

        conversion_matrix_out.AddMember("y_range", set_range(allocator, conversion_matrix_in.y_range).Move(), allocator);
        conversion_matrix_out.AddMember("u_range", set_range(allocator, conversion_matrix_in.u_range).Move(), allocator);
        conversion_matrix_out.AddMember("v_range", set_range(allocator, conversion_matrix_in.v_range).Move(), allocator);

        conversion_matrix_out.AddMember("y_packed_range", set_range(allocator, conversion_matrix_in.y_packed_range).Move(), allocator);
        conversion_matrix_out.AddMember("u_packed_range", set_range(allocator, conversion_matrix_in.u_packed_range).Move(), allocator);
        conversion_matrix_out.AddMember("v_packed_range", set_range(allocator, conversion_matrix_in.v_packed_range).Move(), allocator);

        return conversion_matrix_out;
    }

    static rapidjson::Value set_image_origin(Allocator & allocator, xyuv::image_origin image_origin_in) {
        switch (image_origin_in) {
            case xyuv::image_origin::LOWER_LEFT:
                return rapidjson::Value("lower_left", allocator);
            case xyuv::image_origin::UPPER_LEFT:
                return rapidjson::Value("upper_left", allocator);
            default:
                XYUV_ASSERT(false);
        }
    }

    static rapidjson::Value set_sample(Allocator & allocator, const xyuv::sample & sample_in) {
        rapidjson::Value sample_out(rapidjson::kObjectType);

        sample_out.AddMember("plane", rapidjson::Value(static_cast<uint32_t>(sample_in.plane)).Move(), allocator);
        sample_out.AddMember("integer_bits", rapidjson::Value(static_cast<uint32_t>(sample_in.integer_bits)).Move(), allocator);
        sample_out.AddMember("fractional_bits", rapidjson::Value(static_cast<uint32_t>(sample_in.fractional_bits)).Move(), allocator);

        sample_out.AddMember("offset", rapidjson::Value(static_cast<uint32_t>(sample_in.offset)).Move(), allocator);
        sample_out.AddMember("has_continuation", rapidjson::Value(sample_in.has_continuation).Move(), allocator);

        return sample_out;
    }

    static rapidjson::Value set_samples(Allocator & allocator, const std::vector<xyuv::sample> & samples_in) {
        rapidjson::Value samples_out(rapidjson::kArrayType);
        for (const auto & sample : samples_in) {
            samples_out.PushBack(set_sample(allocator, sample).Move(), allocator);
        }

        return samples_out;
    }

    static rapidjson::Value set_channel_block(Allocator & allocator, const xyuv::channel_block & channel_block_in) {
        rapidjson::Value channel_block_out(rapidjson::kObjectType);

        channel_block_out.AddMember("w", rapidjson::Value(static_cast<uint32_t>(channel_block_in.w)).Move(), allocator);
        channel_block_out.AddMember("h", rapidjson::Value(static_cast<uint32_t>(channel_block_in.w)).Move(), allocator);

        channel_block_out.AddMember("samples", set_samples(allocator, channel_block_in.samples).Move(), allocator);

        return channel_block_out;
    }

    std::string format_to_json(const xyuv::format &format) {
        rapidjson::Document document(rapidjson::kObjectType);

        Allocator & allocator = document.GetAllocator();

        document.AddMember("fourcc", rapidjson::Value(format.fourcc.c_str(), allocator).Move(), allocator);
        document.AddMember("image_w", rapidjson::Value(format.image_w).Move(), allocator);
        document.AddMember("image_h", rapidjson::Value(format.image_h).Move(), allocator);
        document.AddMember("size", rapidjson::Value(format.size).Move(), allocator);

        document.AddMember("origin", set_image_origin(allocator, format.origin).Move(), allocator);

        document.AddMember("chroma_siting", set_chroma_siting(allocator, format.chroma_siting).Move(), allocator);
        document.AddMember("conversion_matrix", set_conversion_matrix(allocator, format.conversion_matrix).Move(), allocator);

        document.AddMember("planes", set_planes(allocator, format.planes).Move(), allocator);

        document.AddMember("y_block", set_channel_block(allocator, format.channel_blocks[xyuv::channel::Y]).Move(), allocator);
        document.AddMember("u_block", set_channel_block(allocator, format.channel_blocks[xyuv::channel::U]).Move(), allocator);
        document.AddMember("v_block", set_channel_block(allocator, format.channel_blocks[xyuv::channel::V]).Move(), allocator);
        document.AddMember("a_block", set_channel_block(allocator, format.channel_blocks[xyuv::channel::A]).Move(), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);

        std::string resultant_json = buffer.GetString();

        return resultant_json;
    }
}
