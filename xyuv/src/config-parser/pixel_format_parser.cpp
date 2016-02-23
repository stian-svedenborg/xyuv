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

#include "../config_parser.h"
#include "xyuv/structures/format_template.h"
#include "xyuv/structures/constants.h"
#include "../to_string.h"
#include "parsing_helpers.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <unordered_map>
#include <iostream>
#include <limits>

namespace xyuv {

#define DECORATE(name, stmts) \
do {\
    try {\
        stmts;\
    } catch (parse_error & e) {\
        throw parse_error(std::string(e.what()) + " while parsing '" + name + "'");\
    }\
} while (0)

class format_template_parser {
public:
    format_template_parser();

    void parse(const std::string &json, format_template *out);

private:
    void parse_document(rapidjson::Document &root, format_template *out);

    // Top-level:
    void parse_fourcc(rapidjson::Value *licence_root, format_template *out);

    void parse_subsampling(rapidjson::Value *licence_root, format_template *out);

    void parse_origin(rapidjson::Value *licence_root, format_template *out);

    // Parsing of planes.
    void parse_block_order(rapidjson::Value *plane_root, block_order *block_order);

    void parse_plane(rapidjson::Value *plane_root, plane_template *plane);

    // Parsing of Channels
    void parse_block(rapidjson::Value *channel_root, channel_block *block);

    void parse_sample(rapidjson::Value *sample_root, channel_block *block);

    // Conversion helpers
    std::unordered_map<std::string, image_origin> str_to_origin;
};

xyuv::format_template_parser::format_template_parser() :
        str_to_origin({
                              {"upper_left", image_origin::UPPER_LEFT},
                              {"lower_left", image_origin::LOWER_LEFT}
                      }) { }

void format_template_parser::parse(const std::string &json, format_template *out) {
    rapidjson::Document d;
    d.Parse(json.c_str());

    if (d.HasParseError()) {
        rapidjson::ParseResult res(d.GetParseError(), d.GetErrorOffset());
        throw parse_error(std::string("on line ") + to_string(line_number_from_offset(json, res.Offset())) +
                          " JSON syntax error: " + rapidjson::GetParseError_En(res.Code()));
    }

    format_template tmp;

    parse_document(d, &tmp);

    *out = tmp;
}

void format_template_parser::parse_document(rapidjson::Document &root, format_template *out) {
    DECLARE_REQUIRED(root, origin, String);
    DECLARE_REQUIRED(root, subsampling_mode, Object);

    DECLARE_REQUIRED(root, y_block, Object);
    DECLARE_REQUIRED(root, u_block, Object);
    DECLARE_REQUIRED(root, v_block, Object);
    DECLARE_REQUIRED(root, a_block, Object);

    DECLARE_REQUIRED(root, planes, Array);

    DECLARE_OPTIONAL(root, fourcc, String);

    parse_origin(origin, out);

    DECORATE("subsampling", parse_subsampling(subsampling_mode, out));


    DECORATE("y_block", parse_block(y_block, &out->channel_blocks[static_cast<uint32_t>(channel::Y)]));
    DECORATE("u_block", parse_block(u_block, &out->channel_blocks[static_cast<uint32_t>(channel::U)]));
    DECORATE("v_block", parse_block(v_block, &out->channel_blocks[static_cast<uint32_t>(channel::V)]));
    DECORATE("a_block", parse_block(a_block, &out->channel_blocks[static_cast<uint32_t>(channel::A)]));

    int i = 0;
    for (auto it = planes->Begin(); it != planes->End(); ++it) {
        plane_template plane;
        DECORATE("planes[" + to_string(i) + "]", parse_plane(&(*it), &plane));

        out->planes.push_back(plane);
        i++;
    }

    if (fourcc) {
        DECORATE("fourcc", parse_fourcc(fourcc, out));
    }
}

void format_template_parser::parse_fourcc(rapidjson::Value *fourcc, format_template *out) {
    if (fourcc->GetStringLength() > 4) {
        throw parse_error("Length of field 'fourcc' must be <= 4 characters.");
    }

    out->fourcc = std::string(fourcc->GetString(), fourcc->GetStringLength());
}

void format_template_parser::parse_subsampling(rapidjson::Value *subsampling_root, format_template *out) {
    DECLARE_REQUIRED(*subsampling_root, macro_px_w, Uint);
    DECLARE_REQUIRED(*subsampling_root, macro_px_h, Uint);

    VALIDATE_RANGE(0, 255, macro_px_w, Uint);
    VALIDATE_RANGE(0, 255, macro_px_h, Uint);
    out->subsampling.macro_px_w = static_cast<uint8_t>(macro_px_w->GetUint());
    out->subsampling.macro_px_h = static_cast<uint8_t>(macro_px_h->GetUint());
}

void format_template_parser::parse_origin(rapidjson::Value *origin, format_template *out) {
    auto it = str_to_origin.find(std::string(origin->GetString(), origin->GetStringLength()));
    if (it == str_to_origin.end()) {
        throw parse_error("Origin string not recognized.");
    }

    out->origin = it->second;
}

static void parse_block_order_swizzle(rapidjson::Value *array, uint8_t (&out)[32] ) {
    std::vector<uint8_t> swizzles;
    for (auto it = array->Begin(); it != array->End(); ++it) {
        if (it->IsString() && std::string{ it->GetString(), it->GetStringLength() } == "-") {
            swizzles.push_back(block_order::NOT_USED);
        }
        else {
            if (!it->IsUint()) {
                throw parse_error("Block order bit swizzle not recognized. Expected integer or '-'.");
            }
            uint32_t val = it->GetUint();
            if (val >= 32) {
                throw parse_error("Block order bit swizzle out or range, must be between 0 and 31. (Or '-' if unused.)");
            }
            swizzles.push_back(static_cast<uint8_t>(val));
        }
    }

    if (swizzles.size() >= 32) {
        throw parse_error("Block order bit swizzles are only supported up to 32 bit swizzle patterns. The given pattern is too big.");
    }

    // The bit pattern is stored with the least significant bit first in the array. So we need to reverse the vector.
    uint32_t i = 0;
    for (auto rit = swizzles.rbegin(); rit != swizzles.rend(); ++rit) {
        out[i++] = *rit;
    }
}

void format_template_parser::parse_block_order(rapidjson::Value *block_order_root, block_order *block_order) {
    DECLARE_REQUIRED(*block_order_root, mega_block_width, Uint);
    DECLARE_REQUIRED(*block_order_root, mega_block_height, Uint);
    DECLARE_REQUIRED(*block_order_root, x_mask, Array);
    DECLARE_REQUIRED(*block_order_root, y_mask, Array);

    block_order->mega_block_width = mega_block_width->GetUint();
    block_order->mega_block_height = mega_block_height->GetUint();

    DECORATE("x_mask", parse_block_order_swizzle(x_mask, block_order->x_mask));
    DECORATE("y_mask", parse_block_order_swizzle(y_mask, block_order->y_mask));
}

static xyuv::interleave_pattern interleavePatternParser(rapidjson::Value* val) {
    std::string str(val->GetString());
    if (str == "NO_INTERLEAVING") {
        return xyuv::interleave_pattern::NO_INTERLEAVING;
    }
    if (str == "INTERLEAVE_1_3_5__0_2_4") {
        return xyuv::interleave_pattern::INTERLEAVE_1_3_5__0_2_4;
    }
    if (str == "INTERLEAVE_0_2_4__1_3_5") {
        return xyuv::interleave_pattern::INTERLEAVE_0_2_4__1_3_5;
    }
    throw parse_error("Interleave pattern not recognized, must be one of: NO_INTERLEAVING, INTERLEAVE_1_3_5__0_2_4 or INTERLEAVE_0_2_4__1_3_5");
}

void format_template_parser::parse_plane(rapidjson::Value *plane_root, plane_template *plane) {
    DECLARE_REQUIRED(*plane_root, base_offset, String);
    DECLARE_REQUIRED(*plane_root, line_stride, String);
    DECLARE_REQUIRED(*plane_root, plane_size, String);
    DECLARE_REQUIRED(*plane_root, block_stride, Uint);
    DECLARE_OPTIONAL(*plane_root, interleave_pattern, String);
    DECLARE_OPTIONAL(*plane_root, block_order, Object);

    plane->base_offset_expression = base_offset->GetString();
    plane->line_stride_expression = line_stride->GetString();
    plane->plane_size_expression = plane_size->GetString();
    plane->block_stride = block_stride->GetUint();
    if (interleave_pattern) {
        plane->interleave_mode = interleavePatternParser(interleave_pattern);
    }
    else {
        plane->interleave_mode = xyuv::interleave_pattern::NO_INTERLEAVING;
    }

    if (block_order) {
        DECORATE("block_order", parse_block_order(block_order, &plane->block_order));
    }
}

void format_template_parser::parse_sample(rapidjson::Value *sample_root, channel_block *block) {
    DECLARE_REQUIRED(*sample_root, plane, Uint);
    DECLARE_REQUIRED(*sample_root, int_bits, Uint);
    DECLARE_REQUIRED(*sample_root, frac_bits, Uint);
    DECLARE_REQUIRED(*sample_root, offset, Uint);
    DECLARE_OPTIONAL(*sample_root, has_continuation, Bool);

    VALIDATE_RANGE(0, 255, plane, Uint);
    VALIDATE_RANGE(0, 255, int_bits, Uint);
    VALIDATE_RANGE(0, 255, frac_bits, Uint);
    VALIDATE_RANGE(0, std::numeric_limits<uint16_t>::max(), offset, Uint);

    xyuv::sample sample;
    sample.plane = static_cast<uint8_t>(plane->GetUint());
    sample.integer_bits = static_cast<uint8_t>(int_bits->GetUint());
    sample.fractional_bits = static_cast<uint8_t>(frac_bits->GetUint());
    sample.offset = static_cast<uint16_t>(offset->GetUint());

    if (has_continuation) {
        sample.has_continuation = has_continuation->GetBool();
    } else {
        sample.has_continuation = false;
    }

    block->samples.push_back(sample);
}

void format_template_parser::parse_block(rapidjson::Value *channel_root, channel_block *block) {
    DECLARE_REQUIRED(*channel_root, block_w, Uint);
    DECLARE_REQUIRED(*channel_root, block_h, Uint);
    DECLARE_REQUIRED(*channel_root, samples, Array);

    VALIDATE_RANGE(0, std::numeric_limits<uint16_t>::max(), block_w, Uint);
    VALIDATE_RANGE(0, std::numeric_limits<uint16_t>::max(), block_h, Uint);

    block->w = static_cast<uint16_t>(block_w->GetUint());
    block->h = static_cast<uint16_t>(block_h->GetUint());

    auto it = samples->Begin();
    auto end = samples->End();
    int i = 0;
    for (; it != end; ++it) {
        DECORATE("samples[" + to_string(i) + "]", parse_sample(&(*it), block));
        i++;
    }
}

xyuv::format_template parse_format_template(const std::string &json) {
    xyuv::format_template format_template;
    format_template_parser().parse(json, &format_template);
    return format_template;
}

} // namespace xyuv