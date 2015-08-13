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
#include <string>

namespace xyuv {

struct chroma_siting;
struct format_template;
struct conversion_matrix;

/** \brief Read a configuration file stripping it of all comments.
 * \details JSON does not support comments, I thought this was a shame,
 *  so the configuration files in xyuv do support comments (lines starting with #)
 *  the comments are then stripped away bu this function before being passed on to
 *  the parser functions. */
extern std::string read_json(const std::string &filename);

/** Parse a chroma siting configuration json string.*/
extern chroma_siting parse_chroma_siting(const std::string &json);

/** Parse a pixel format template from a json string. */
extern format_template parse_format_template(const std::string &json);

/** Parse an rgb conversion matrix from a json string. */
extern conversion_matrix parse_conversion_matrix(const std::string &json);

} // namespace xyuv