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
#include "parsing_helpers.h"
#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include "parse_error.h"
#include <rapidjson/error/en.h>
#include "xyuv/structures/chroma_siting.h"
#include "../to_string.h"

namespace xyuv {

//! \brief Parses a sample point.
//! \throw parse_error on error.
static std::pair<float, float> parse_sample_point(rapidjson::Value &root);

//! \brief parse a chroma siting descriptor from a valid json string.
//! \throw parse_error on error.
chroma_siting parse_chroma_siting(const std::string &json) {
    rapidjson::Document d;

    d.Parse(json.c_str());

    if (d.HasParseError()) {
        rapidjson::ParseResult res(d.GetParseError(), d.GetErrorOffset());
        throw parse_error(std::string("on line ") + to_string(line_number_from_offset(json, res.Offset())) +
                          " JSON syntax error: " + rapidjson::GetParseError_En(res.Code()));
    }

    DECLARE_REQUIRED(d, block_width, Uint);
    DECLARE_REQUIRED(d, block_height, Uint);
    DECLARE_REQUIRED(d, u_sample_point, Object);
    DECLARE_REQUIRED(d, v_sample_point, Object);

    xyuv::chroma_siting siting;
    VALIDATE_RANGE(0, 255, block_width, Uint);
    VALIDATE_RANGE(0, 255, block_height, Uint);
    siting.subsampling.macro_px_w = static_cast<uint8_t>(block_width->GetUint());
    siting.subsampling.macro_px_h = static_cast<uint8_t>(block_height->GetUint());

    // Look for U and V sample points.
    try {
        siting.u_sample_point = parse_sample_point(*u_sample_point);
    } catch (parse_error &e) {
        throw parse_error(std::string(e.what()) + " while parsing 'u_sample_point'");
    }

    try {
        siting.v_sample_point = parse_sample_point(*v_sample_point);
    } catch (parse_error &e) {
        throw parse_error(std::string(e.what()) + " while parsing 'v_sample_point'");
    }

    return siting;
}


static std::pair<float, float> parse_sample_point(rapidjson::Value &root) {
    DECLARE_REQUIRED(root, sample_x, Double);
    DECLARE_REQUIRED(root, sample_y, Double);

    VALIDATE_RANGE(0.0, 1.0, sample_x, Double);
    VALIDATE_RANGE(0.0, 1.0, sample_y, Double);

    return std::make_pair(static_cast<float>(sample_x->GetDouble()), static_cast<float>(sample_y->GetDouble()));
}

// TODO: Add validation function.
// TODO: Add convert to cpp function.

} // namespace xyuv