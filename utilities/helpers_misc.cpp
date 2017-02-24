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

void Helpers::PrintAllFormats(const xyuv::config_manager &manager) {

        std::cout << "Available format templates:\n";
        std::cout << "  | Key    |  Subsampling |\n";
        for (const auto & fmt : manager.get_format_templates()) {
            std::cout   << "    " << std::left << std::setw(10) << fmt.first
                        << "  <- " << static_cast<int>(fmt.second.subsampling.macro_px_w)
                        << 'x' << static_cast<int>(fmt.second.subsampling.macro_px_h) << '\n';
        }

        std::cout << "Available chroma sitings:\n";
        std::cout << "  | Key    |  Subsampling |\n";
        for (const auto & siting : manager.get_chroma_sitings()) {
            std::cout   << "    " << std::left << std::setw(10) << siting.first
                        << "  <- " << static_cast<int>(siting.second.subsampling.macro_px_w)
                        << 'x' << static_cast<int>(siting.second.subsampling.macro_px_h) << '\n';
        }

        std::cout << "Available conversion matrices:\n";
        std::cout << "  | Key           |\n";
        for (const auto & conv : manager.get_conversion_matrices()) {
            std::cout   << "    " << std::left << std::setw(10) << conv.first << '\n';
        }

        std::cout << std::endl;
}

std::string Helpers::ToLower(const std::string& str) {
    std::string result;
    for (const char & c : str ) {
        result.push_back(static_cast<char>(std::tolower(c)));
    }
    return result;
}