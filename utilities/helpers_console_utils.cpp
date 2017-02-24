/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Stian Valentin Svedenborg
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

#include "helpers.h"

#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <sstream>
#include "console_width.h"

constexpr int FALLBACK_COLUMN_WIDTH = 80;
constexpr int MAX_COLUMN_WIDTH = 150;
constexpr int INDENT_WIDTH = 8;

int Helpers::GetAdaptedConsoleWidth() {
    int console_width = get_console_width();
    if (console_width == 0) {
        console_width = FALLBACK_COLUMN_WIDTH;
    } else if (console_width > MAX_COLUMN_WIDTH) {
        console_width = MAX_COLUMN_WIDTH;
    }
    return console_width;
}

std::string Helpers::FormatString(uint32_t indent, uint32_t max_width, const std::string &str) {
    std::size_t current_line_length = 0;
    std::string str_indent(indent, ' ');
    std::string result = str_indent;
    // Consume characters from input:
    std::string word;
    for (char c : str) {
        bool push_word = false;
        bool push_line = false;
        switch (c) {
            case '\n':
                push_line = true;
            case ' ':
                push_word = true;
                break;
            default:
                word += c;
                break;
        }

        if (push_word) {
            if (current_line_length + word.size() + 1 > max_width) {
                result += "\n" + str_indent;
                current_line_length = 0;
            }

            result += word + ' ';
            current_line_length += word.size() + 1;
            word.clear();

            if (push_line) {
                result += "\n" + str_indent;
                current_line_length = 0;
            }
        }
    }
    // Also push the last word:
    result += word;

    return result;
}

void Helpers::PrintHelpSection(const std::string &short_arg, const std::string &long_arg, const std::string &helpstring) {
    int console_width = GetAdaptedConsoleWidth();

    console_width -= INDENT_WIDTH;

    std::cout << short_arg << std::endl << long_arg << std::endl;
    std::cout << FormatString(INDENT_WIDTH, console_width, helpstring) << '\n' << std::endl;

}

