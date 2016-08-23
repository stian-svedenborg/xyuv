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
#include <fstream>
#include <sys/errno.h>
#include <stdexcept>

namespace xyuv {

std::string read_json(const std::string &filename) {
    std::ifstream fin(filename);

    if (!fin) {
        throw std::runtime_error("Could not open file '" + filename + "'");
    }

    std::string result;
    std::string line;

    while (std::getline(fin, line)) {
        // Strip leading whitespace.
        const char *start = line.c_str();
        while (iswspace(*start)) {
            ++start;
        }
        // if line starts with a # it is a comment, ignore.
        if (*start == '#') {
            result += '\n'; // Add the newline to keep line numbers coherent.
            continue;
        }

        result += start;
        result += " \n";

    }

    return result;
}

} // namespace xyuv