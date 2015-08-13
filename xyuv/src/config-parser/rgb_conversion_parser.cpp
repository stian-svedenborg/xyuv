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
#include <rapidjson/error/en.h>
#include "xyuv/structures/conversion_matrix.h"
#include "../to_string.h"

namespace xyuv {

class conversion_matrix_parser {
public:
static void parse(const std::string& json, conversion_matrix *out) {
    rapidjson::Document d;

    d.Parse(json.c_str());

    if (d.HasParseError()) {
        rapidjson::ParseResult res(d.GetParseError(), d.GetErrorOffset());
        throw parse_error(std::string("on line ")+ to_string(line_number_from_offset(json, res.Offset())) + " JSON syntax error: " + rapidjson::GetParseError_En(res.Code()));
    }

    DECLARE_REQUIRED(d, to_yuv, Array);
    DECLARE_REQUIRED(d, to_rgb, Array);

    DECLARE_REQUIRED(d, y_range, Array);
    DECLARE_REQUIRED(d, u_range, Array);
    DECLARE_REQUIRED(d, v_range, Array);

    DECLARE_REQUIRED(d, y_packed_range, Array);
    DECLARE_REQUIRED(d, u_packed_range, Array);
    DECLARE_REQUIRED(d, v_packed_range, Array);

    conversion_matrix tmp;

    try {
        parse_matrix(to_yuv, tmp.rgb_to_yuv);
    } catch (parse_error & e) {
        throw parse_error( e.what() + std::string(" while parsing 'to_yuv' matrix"));
    }
    try {
	    parse_matrix(to_rgb, tmp.yuv_to_rgb);
    } catch (parse_error & e) {
        throw parse_error( e.what() + std::string(" while parsing 'to_rgb' matrix"));
    }

    // Ranges.
    #define PARSE_RANGE(name) \
    try {\
        tmp.name = parse_range_float(name);\
    } catch (parse_error & e) {\
        throw parse_error( e.what() + std::string(" while parsing '" # name "' matrix"));\
    }
    PARSE_RANGE(y_range);
    PARSE_RANGE(u_range);
    PARSE_RANGE(v_range);

    PARSE_RANGE(y_packed_range);
    PARSE_RANGE(u_packed_range);
    PARSE_RANGE(v_packed_range);

    #undef PARSE_RANGE
    *out = tmp;

}

private:


static void parse_matrix(rapidjson::Value* root, float *matrix) {
    auto it = root->Begin();
    const auto end = root->End();

    uint32_t counter = 0;
    for (int i = 0; i < 3; i++) {

		if (it == end) {
            throw parse_error("Conversion matrix must have exactly three rows.");
		}
		if ( !it->IsArray() ) {
            throw parse_error("Row[" + to_string(i) + "] wrong type. Expected array.");
        }

        auto inner_it = it->Begin();
        const auto inner_end = it->End();

        for (int j = 0; j < 3; j++) {
			if (inner_it == inner_end) {
                throw parse_error("Row[" + to_string(i) + "] must have exactly 3 elements.");
			}
			if (!inner_it->IsDouble()) {
                throw parse_error("Matrix[" + to_string(i) + "]["  + to_string(j) + "] : Expected double.");
			}

            matrix[counter++] = static_cast<float>(inner_it->GetDouble());

            ++inner_it;
        }
        if (inner_it != inner_end) {
            throw parse_error("Row[" + to_string(i) + "] must have exactly 3 elements.");
        }
        ++it;
    }

    if (it != end || counter != 9) {
        throw parse_error("Conversion matrix must be a 3x3 matrix.");
    }
}

    static std::pair<float,float> parse_range_float(rapidjson::Value* root) {
        if (root->Size() != 2) {
            throw parse_error("Range field must have exactly two elements." );
        }

        if (!(*root)[0].IsDouble()) {
            throw parse_error("Range low: Expected double.");
        }

        if (!(*root)[1].IsDouble()) {
            throw parse_error("Range high: Expected double.");
        }

        std::pair<float,float> range;
        range.first = static_cast<float>((*root)[0].GetDouble());
        range.second = static_cast<float>((*root)[1].GetDouble());

        if (range.second < range.first ) {
            DBG_MSG("WARNING: Range field is wrong, min > max. Swapping.");
            using std::swap;
            swap(range.first, range.second);
        }

        return range;
    }
};

conversion_matrix parse_conversion_matrix(const std::string& json) {
    xyuv::conversion_matrix conversion_matrix;
    conversion_matrix_parser::parse(json, &conversion_matrix);
    return conversion_matrix;
}

} // namespace xyuv